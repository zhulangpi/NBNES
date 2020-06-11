#include "ppu.h"


unsigned char *pattern_tbl0;
unsigned char *pattern_tbl1;


unsigned char ppu_reg[PPU_REG_MAX];
unsigned char ppu_reg_OAMDMA;

void ppu_addr(unsigned short addr)
{


    if(addr >= 0x4000){
        addr &= 0x3FFF;
    }


    if(addr < 0x1000){          //pattern tbl0
        
        
    }else if(addr < 0x2000){    //pattern tbl1
        addr -= 0x1000;

    }else if(addr < 0x3F00){    //name tbl
        addr &= 0x2FFF;
        addr -= 0x2000;
    }else{                      //palettes
        addr &= 0x3F1F;
        addr -= 0x3F00;
    }
        



}











