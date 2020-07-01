#include <stdlib.h>
#include <stdio.h>
#include "cpu.h"
#include "ppu.h"
#include "display.h"


struct pattern_tbl *pattern_tbl0;
struct pattern_tbl *pattern_tbl1;
struct bg_tbl       *bg[4];
unsigned char image_palette[0x10];
unsigned char sprite_palette[0x10];
unsigned int screen_color_idx[WIDTH*HEIGHT];

unsigned char ppu_SPRRAM[0x100];


unsigned char ppu_reg_PPUCTRL;
unsigned char ppu_reg_PPUMASK;
unsigned char ppu_reg_PPUSTATUS;
unsigned char ppu_reg_OAMADDR;
unsigned char ppu_reg_OAMDATA;
unsigned char ppu_reg_PPUSCROLL_X, ppu_reg_PPUSCROLL_Y;
unsigned short ppu_reg_PPUADDR;
unsigned char ppu_reg_PPUDATA;

unsigned char ppu_reg_OAMDMA;


int PPUDATA_first_read;
int ppu_ready;
unsigned char ppu_latch, ppu_addr_latch;

int scroll_writex2 = 0, addr_writex2 = 0;

int scanline = 0;


void p_pal(void)
{
    int i;
    for(i=0;i<0x10;i++){
        printf("%#x\n", image_palette[i]);
    }
    printf("\n");
    for(i=0;i<0x10;i++){
        printf("%#x\n", sprite_palette[i]);
    }
}


unsigned char ppu_addr(unsigned short addr, unsigned char data, int rw)
{
    unsigned char *v;

    //printf("PPU: %s: %#x\n", (rw==CPU_RD)?"read":"write", addr);

    if(addr >= 0x4000){
        addr &= 0x3FFF;
    }else if(addr >= 0x3F20){
        addr &= 0x3F1F;
    }

    if(addr < 0x1000){          //pattern tbl0
        v = (unsigned char*)pattern_tbl0;
        
    }else if(addr < 0x2000){    //pattern tbl1
        addr -= 0x1000;
        v = (unsigned char*)pattern_tbl1;
    }else if(addr < 0x3F00){    //name tbl
        addr &= 0x0FFF;
        v = (unsigned char *)bg[addr>>10];
        if(rw==CPU_WRT){
            v[addr] = data;
        }
        if( addr == 0x82){
            printf("0x2082: %#x\n", v[addr]);
        }
    }else if(addr < 0x4000){                      //palettes
        //printf("PPU: %s: %#x %#x\n", (rw==CPU_RD)?"read":"write", addr, data);
        addr &= 0x3F1F;
        if(addr < 0x3F10){
            v = (unsigned char *)image_palette;
        }else{
            v = (unsigned char *)sprite_palette;
        }
        addr &= 0xF;
        if(rw==CPU_WRT){
            if(addr == 0x4 || addr == 0x08 || addr == 0x0c){
                return v[addr];
            }else if(addr == 0){
                v[0]=v[4]=v[8]=v[0xc]=data;       
            }else{
				v[addr] = data;
			}
        }

    }



    return v[addr];
}


unsigned char ppu_reg_rw(unsigned short addr, unsigned char data, int rw)
{
    unsigned char ret;

    if(rw==CPU_WRT){
        ppu_latch = data;
    }

    ppu_reg_PPUADDR &= 0x3FFF;
    switch(addr & 0x7){
    case PPUCTRL:
        if((rw==CPU_WRT) && ppu_ready){
            ppu_reg_PPUCTRL = data;
        }
        break;
    case PPUMASK:
        if((rw==CPU_WRT) && ppu_ready){
            ppu_reg_PPUMASK = data;
        }
        break;
    case PPUSTATUS:
        if(rw==CPU_RD){
            ret = ppu_reg_PPUSTATUS;
            ppu_reg_PPUSTATUS &= ~VBlank;
            ppu_reg_PPUSTATUS &= ~Sprite_0_hit;
            ppu_latch = ret;
            ppu_addr_latch = 0;
            PPUDATA_first_read = 1;
            scroll_writex2 = 0;
            addr_writex2 = 0;
        }
        break;
    case OAMADDR:
        if(rw==CPU_WRT){
            ppu_reg_OAMADDR = data;
        }
        break;
    case OAMDATA:
        if(rw==CPU_RD)
            ppu_latch = ppu_SPRRAM[ppu_reg_OAMADDR];
        else if(rw==CPU_WRT){
            ppu_SPRRAM[ppu_reg_OAMADDR++] = data;
        }
        break;
    case PPUSCROLL:
        if(rw==CPU_WRT){
            if(scroll_writex2)
                ppu_reg_PPUSCROLL_Y = data;
            else
                ppu_reg_PPUSCROLL_X = data;
            scroll_writex2 = !scroll_writex2;
        }
        break;
    case PPUADDR:
        if(rw==CPU_WRT){
            if(!ppu_ready)
                return 0;
            if(addr_writex2){
                ppu_reg_PPUADDR = (((unsigned short)ppu_addr_latch) << 8)| data;
            }else{
                ppu_addr_latch = data;
            }
            addr_writex2 = !addr_writex2;
            PPUDATA_first_read = 1;
            //printf("ppu_reg_ADDR:%#x, writex2:%#x\n", ppu_reg_ADDR, addr_writex2);
        }
        break;
    case PPUDATA:
        ret = ppu_addr(ppu_reg_PPUADDR, data, rw);
        //printf("ppu_addr: %#x\n", ppu_reg_ADDR);
        if(rw==CPU_RD){
            if(ppu_reg_PPUADDR < 0x3F00){
                ppu_latch = ret;
            }else{
                ppu_latch = 0;
            }
            if(PPUDATA_first_read){
                PPUDATA_first_read = 0;
            }
        }
        ppu_reg_PPUADDR += (ppu_reg_PPUCTRL & VRAM_INC) ? 32 : 1;
        break;
    default:
        break;
    }
    if(rw==CPU_WRT)
        ppu_latch = data;
    return ret;
}


// 4 tile 16*16, 16*15
//  tile num to attri tbl num, 960 map 64
// get the bit[3:2] of color idx
unsigned int tile2attri(unsigned int tile)
{
    int blockx, blocky, tilex, tiley, idx;
    unsigned char attri;
    blockx = tile / 16;
    blocky = tile % 16;
    attri = bg[0]->attri_tbl[blockx + blocky*BLK_PER_ROW];

    tilex = tile / 32;
    tiley = tile % 32;

    idx = (tilex%2) | ((tiley%2)<<1);
    attri >>= (2*idx);
    attri &= 0x03;
    return attri;
}


// 960 tiles (8*8) pixels
void tile2pixel(int tile, unsigned int *screen)
{
    int x,y,i,j,idx;
    struct chr tmp;
    unsigned int bitmask;

    y = tile / TILE_PER_ROW;
    x = tile - y*TILE_PER_ROW;
    //printf( "tile:%d x:%d y:%d\n", tile, x, y);
    tmp = pattern_tbl0->c[bg[0]->name_tbl[tile]];
    for(i=7; i>=0; i--){
        for(j=0;j<8;j++){
            idx = (8*y+i)*WIDTH + 8*x + j;
            bitmask = 1<<(7-j);
            //bit[1:0]
            screen[idx] = (!!(tmp.bit0[i] & bitmask)) | (!!(tmp.bit1[i] & bitmask)<<1) ;
            //bit[3:2]
            screen[idx] |= tile2attri(tile)<<2 ;
            //printf("%d %d %d %#x\n",i,j,idx, screen[idx] );
        }
    }
}


void bg_render(void)
{
    int i;

    for(i=0;i<960;i++){
        tile2pixel(i, screen_color_idx);
    }
}


void do_vblank(void)
{
    ppu_reg_PPUSTATUS |= VBlank;
    ppu_reg_PPUSTATUS &= ~Sprite_0_hit;
    if( ppu_reg_PPUCTRL & 0x80 ){
        cpu_NMI();
    }
}

void ppu_run(void)
{
    if(!ppu_ready && cpu_cycle() > 29685){
        ppu_ready = 1;
    }
    scanline++;
    //printf("scanline: %d\n",scanline);
    if(ppu_reg_PPUMASK & 0x40){

    }
    if(scanline == 241){
        do_vblank();
    }else if(scanline == 262){
        scanline = -1;
        ppu_reg_PPUSTATUS &= ~VBlank;
        //update screen
        bg_render();
        flush_screen();
    }
}


void ppu_init(void)
{
    bg[0] = (struct bg_tbl*)malloc(sizeof(struct pattern_tbl));
    bg[1] = (struct bg_tbl*)malloc(sizeof(struct pattern_tbl));
    bg[2] = (struct bg_tbl*)malloc(sizeof(struct pattern_tbl));
    bg[3] = (struct bg_tbl*)malloc(sizeof(struct pattern_tbl));

    ppu_reg_PPUCTRL = 0;
    ppu_reg_PPUMASK = 0;
    ppu_reg_PPUSTATUS = 0xA0;
    ppu_reg_OAMADDR = 0;
    ppu_reg_OAMDATA = 0;
    ppu_reg_PPUSCROLL_X = 0;
    ppu_reg_PPUSCROLL_Y = 0;
    ppu_reg_PPUADDR = 0;
    ppu_reg_PPUDATA = 0;
    
    ppu_ready = 0;
    PPUDATA_first_read = 1;
}

