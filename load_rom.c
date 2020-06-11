#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "load_rom.h"

struct ines_header *i_h;

int load_rom(char* nes_file)
{
    FILE *f;
    int offset, prg_sz, chr_sz;
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

    prg_sz = i_h->prg_rom_sz << (4+10);
    chr_sz = i_h->chr_rom_sz << (3+10);

    printf("prg_sz:%#x chr_sz:%#x\n", prg_sz, chr_sz);
    prg_rom = (char*)malloc(prg_sz);
    chr_rom = (char*)malloc(chr_sz);
    if(!(prg_rom && chr_rom)){
        printf("error: no mem\n");
        return -1;
    }
    
    fseek(f, offset, SEEK_SET);
    fread(prg_rom, sizeof(char), prg_sz, f);
    fseek(f, offset + prg_sz, SEEK_SET);
    fread(chr_rom, sizeof(char), chr_sz, f);
    fclose(f);
    printf("prg_rom[0]:%#x chr_rom[0]:%#x\n",prg_rom[0], chr_rom[0]);
    return 0;
}


