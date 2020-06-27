#include <stdio.h>
#include "load_rom.h"
#include "cpu.h"
#include "ppu.h"
#include "display.h"

extern void p_pal(void);


void p_pat(void)
{
    int i;
    unsigned char *v;
    v = (unsigned char*)pattern_tbl0;
    for(i=0;i<0x1000;i++){
        printf("%#x: %#x\n", i,v[i]);
    }
}

int main(int argc, char* argv[])
{
    int i,j;

    if(load_rom("nestest.nes")<0){
//    if(load_rom("Donkey_Kong.nes")<0){
        printf("error: load nes file failed\n");
    }
    cpu_init();
    ppu_init();
    display_init();

    while(1){
        i = cpu_cycle();
        ppu_run();
        while((cpu_cycle()-i) < (1364/12)){
            cpu_run();
            j++;
            if(j>=40000){
                p_pal();
                p_pat();
                return 0;
            }
        }
    };

    return 0;
}

