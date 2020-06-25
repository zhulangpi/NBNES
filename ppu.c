#include <stdlib.h>
#include <stdio.h>
#include "cpu.h"
#include "ppu.h"


struct pattern_tbl *pattern_tbl0;
struct pattern_tbl *pattern_tbl1;
struct bg_tbl       *bg[4];
unsigned int screen_color_idx[WIDTH*HEIGHT];


unsigned char ppu_reg[PPU_REG_MAX];
unsigned char ppu_reg_OAMDMA;

unsigned short vramaddr;

#define vram_inc()  ((ppu_reg[PPUCTRL] & VRAM_INC) ? 32 : 1)

unsigned char ppu_addr(unsigned short addr, unsigned char data, int rw)
{
    unsigned char *v;

    if(addr >= 0x4000){
        addr &= 0x3FFF;
    }

    if(addr < 0x1000){          //pattern tbl0
        v = (unsigned char*)pattern_tbl0;
        
    }else if(addr < 0x2000){    //pattern tbl1
        addr -= 0x1000;
        v = (unsigned char*)pattern_tbl1;
    }else if(addr < 0x3F00){    //name tbl
        addr &= 0x2FFF;
        addr -= 0x2000;
        v = (unsigned char *)bg[addr>>10];
    }else{                      //palettes
        addr &= 0x3F1F;
        addr -= 0x3F00;
    }

    if(rw==CPU_WRT){
        v[addr] = data;
    }
    return v[addr];
}


unsigned char ppu_reg_rw(unsigned short addr, unsigned char data, int rw)
{
    static int writex2 = 0;
    unsigned char ret;

    addr &= 0x2007;
    addr -= PPUBASE;
    //printf("%s: %x %x\n", (rw==CPU_RD)?"read":"write",PPUBASE+ addr, ppu_reg[addr]);
    switch(addr){
    case PPUCTRL:
        if(rw==CPU_WRT){
            ppu_reg[PPUCTRL] = data;
        }
        break;
    case PPUMASK:
        if(rw==CPU_WRT){
            ppu_reg[PPUMASK] = data;
        }
        break;
    case PPUSTATUS:
        if(rw==CPU_RD){
            ret = ppu_reg[PPUSTATUS];
            ppu_reg[PPUSTATUS] &= ~VBlank;
        }
        break;
    case OAMADDR:
        if(rw==CPU_WRT){
            ppu_reg[OAMADDR] = data;
        }
        break;
    case OAMDATA:
        break;
    case PPUSCROLL:
        writex2 != writex2;
        break;
    case PPUADDR:
        if(writex2==1){
            vramaddr &= 0xFF00;
            vramaddr |= data;
        }else{
            vramaddr &= 0x00FF;
            vramaddr |= (unsigned short)data << 8;
        }
        writex2 != writex2;
        break;
    case PPUDATA:
        ret = ppu_addr(vramaddr, data, rw);
        vramaddr += vram_inc();
        break;
    default:
        break;
    }
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
    for(i=0; i<8; i++){
        for(j=0;j<8;j++){
            idx = (8*y+i)*WIDTH + 8*x + j;
            bitmask = 1<<j;
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

unsigned int idx2palette(unsigned char *idx)
{
    unsigned int pal;

    return 0;
}


void do_vblank(void)
{
    ppu_reg[PPUSTATUS] |= VBlank;
    ppu_reg[PPUSTATUS] &= ~0x40;
    if( ppu_reg[PPUCTRL] & 0x80 ){
        cpu_NMI();
    }
}

void ppu_init(void)
{
    bg[0] = (struct bg_tbl*)malloc(sizeof(struct pattern_tbl));

    ppu_reg[PPUCTRL] = 0;
    ppu_reg[PPUMASK] = 0;
    ppu_reg[PPUSTATUS] = 0xA0;
    ppu_reg[OAMADDR] = 0;
    ppu_reg[OAMDATA] = 0;
    ppu_reg[PPUSCROLL] = 0;
    ppu_reg[PPUADDR] = 0;
    ppu_reg[PPUDATA] = 0;
    

}

