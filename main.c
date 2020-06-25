#include <stdio.h>
#include "load_rom.h"
#include "cpu.h"
#include "ppu.h"
#include "display.h"

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

    j=0;
    while(1){
        i = 10000;
        while(i--){
            cpu_run();
            j++;

            if(j >= 8000)
                return 0;
        }
        //do_vblank();
        //bg_render();
        //nes_flush_buf();
    };

    return 0;
}

