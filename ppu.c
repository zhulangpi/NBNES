#include <stdlib.h>
#include <stdio.h>
#include "ppu.h"


struct pattern_tbl *pattern_tbl0;
struct pattern_tbl *pattern_tbl1;
struct bg_tbl       *bg[4];



unsigned char ppu_reg[PPU_REG_MAX];
unsigned char ppu_reg_OAMDMA;

void ppu_addr(unsigned short addr)
{


    if(addr >= 0x4000){
        addr &= 0x3FFF;
    }


    if(addr < 0x1000){          //pattern tbl0
        pattern_tbl0[addr];
        
    }else if(addr < 0x2000){    //pattern tbl1
        addr -= 0x1000;
        pattern_tbl1[addr];
    }else if(addr < 0x3F00){    //name tbl
        addr &= 0x2FFF;
        addr -= 0x2000;
        bg[addr>>10];


    }else{                      //palettes
        addr &= 0x3F1F;
        addr -= 0x3F00;
    }
    
}


void ppu_init(void)
{
    bg[0] = (struct bg_tbl*)malloc(sizeof(struct pattern_tbl));
}









