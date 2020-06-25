#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "ppu.h"
#include "load_rom.h"

struct ines_header *i_h;
unsigned int mapper;


int load_rom(char* nes_file)
{
    FILE *f;
    int offset, prg_sz, pattern_tbl_sz;
    int i;

    f = fopen(nes_file, "r");   
    if(!f){
        printf("error: open nes file failed\n");
        return -1;
    }

    i_h = (struct ines_header*)malloc(sizeof(struct ines_header));
    fread(i_h, sizeof(struct ines_header), 1, f);
    if(i_h->magic_num!=INES_MAGIC){
        printf("error: not nes file\n");
        return -1;
    }

    offset = sizeof(struct ines_header);
    if(have_trainer(i_h->f6)){
        offset += TRAINER_SZ;
        printf("Have trainer\n");
    }

    mapper = (i_h->f6 & 0xf0) | (i_h->f7 & 0xf0);
    printf("mapper:%d\n", mapper);

    prg_sz = i_h->prg_rom_sz << (4+10);
    pattern_tbl_sz = i_h->pattern_tbl_sz << (3+10);
    printf("prg_sz:%#x pattern_tbl_sz:%#x\n", prg_sz, pattern_tbl_sz);

    prg_rom = (unsigned char*)malloc(prg_sz);

    pattern_tbl0 = (struct pattern_tbl*)malloc(sizeof(struct pattern_tbl));
    if(pattern_tbl_sz > sizeof(struct pattern_tbl)){
        pattern_tbl1 = (struct pattern_tbl*)malloc(sizeof(struct pattern_tbl));
    }
    if(!(prg_rom && pattern_tbl0)){
        printf("error: no mem\n");
        return -1;
    }
    
    fseek(f, offset, SEEK_SET);
    fread(prg_rom, sizeof(char), prg_sz, f);
    fseek(f, offset + prg_sz, SEEK_SET);
    fread(pattern_tbl0, sizeof(char), pattern_tbl_sz, f);
    if(pattern_tbl_sz > sizeof(struct pattern_tbl)){
        fseek(f, offset + sizeof(struct pattern_tbl), SEEK_SET);
        fread(pattern_tbl1, sizeof(char), pattern_tbl_sz-sizeof(struct pattern_tbl), f);
    }

    fclose(f);
    printf("prg_rom[0]:%#x pattern_tbl[0]:%#x\n",prg_rom[0], ((unsigned char*)pattern_tbl0)[0x32-0x10]);

    return 0;
}


