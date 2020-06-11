#include <stdio.h>
#include "load_rom.h"
#include "cpu.h"


int main(int argc, char* argv[])
{
    int i = 9000;

    if(load_rom("nestest.nes")<0){
        printf("error: load nes file failed\n");
    }
    cpu_init();

    while(i--){
        cpu_run();
    }

    return 0;
}

