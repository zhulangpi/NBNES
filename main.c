#include <stdio.h>
#include "load_rom.h"
#include "cpu.h"


int main(int argc, char* argv[])
{
    int i = 50;

    if(load_rom("nestest.nes")<0){
//    if(load_rom("Donkey_Kong.nes")<0){
        printf("error: load nes file failed\n");
    }
    cpu_init();

    while(i--){
        cpu_run();
    }

    return 0;
}

