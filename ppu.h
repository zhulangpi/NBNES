#ifndef _PPU_H
#define _PPU_H


#define WIDTH           (256)
#define HEIGHT          (240)
#define TILE_PER_ROW    (32)
#define TILE_PER_COL    (30)
#define BLK_PER_ROW     (16)
#define BLK_PER_COL     (15)

#define PPUBASE         (0x2000)
#define PPUCTRL         (0)
#define PPUMASK         (1)
#define PPUSTATUS       (2)
#define OAMADDR         (3)
#define OAMDATA         (4)
#define PPUSCROLL       (5)
#define PPUADDR         (6)
#define PPUDATA         (7)
#define PPU_REG_MAX     (8)


#define CPU_addr_OAMDMA      (0x4014)


#define VRAM_INC        (0x04)
#define VBlank          (0x80)
#define Sprite_0_hit    (0x40)



// FROM: http://wiki.nesdev.com/w/index.php/PPU


// 1 screen is consist of 960 tiles
// 256*240 / 8*8(1 tile) = 32*30 = 960 tiles
// each tile is consist of 8*8 pixels


// nes support 64 colors, but it needs 6 bit to index
// nes use Pattern Table save the bit[1:0]
// use Attribute Table save the bit[3:2]
// Attr Tbl use 64B to describe the whole screen,
// every 4 tiles use 1 Attr tbl element, so 960 tiles use 960/4=240 elements
// 64B/2b = 256 elements


// the color of each pixel is indexed by 4 bits,
// so each pixel can have 2^4 = 16 colors, if they use the same Attr tbl element


struct chr{
    unsigned char bit0[8];  // a 8*8 bitmap for tile's pixel bit 0
    unsigned char bit1[8];  // bit 1
};

// pattern tbl is used to save the basic picture(CHR)
// a pattern tbl size is a 256*2*8 = 4096 B 
struct pattern_tbl{
    struct chr c[256];
};


struct bg_tbl{
    unsigned char name_tbl[960];    //960 * 1B, 1B=8b for index the 256 chr
    unsigned char attri_tbl[64];    //64B
};





extern struct pattern_tbl *pattern_tbl0;
extern struct pattern_tbl *pattern_tbl1;
extern struct bg_tbl   *bg[4];
extern unsigned char image_palette[0x10];
extern unsigned char sprite_palette[0x10];


extern unsigned int screen_color_idx[WIDTH*HEIGHT];


extern unsigned char ppu_reg[PPU_REG_MAX];
extern unsigned char ppu_reg_OAMDMA;


extern void ppu_init(void);
extern void bg_render(void);
extern unsigned char ppu_reg_rw(unsigned short, unsigned char, int);
extern void do_vblank(void);
extern void ppu_run(void);
#endif
