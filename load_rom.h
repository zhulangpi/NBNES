#ifndef _LOAD_ROM_H
#define _LOAD_ROM_H


#define INES_MAGIC  0x1a53454e  // 'N' 'E' 'S' 0x1A
struct ines_header{
    unsigned int  magic_num;
    char prg_rom_sz;        //in 16KB units
    char pattern_tbl_sz;    //in 8KB units
    char f6;
    char f7;
    char padding[8];
};

/* flags 6
76543210
||||||||
|||||||+- Mirroring: 0: horizontal (vertical arrangement) (CIRAM A10 = PPU A11)
|||||||              1: vertical (horizontal arrangement) (CIRAM A10 = PPU A10)
||||||+-- 1: Cartridge contains battery-backed PRG RAM ($6000-7FFF) or other persistent memory
|||||+--- 1: 512-byte trainer at $7000-$71FF (stored before PRG data)
||||+---- 1: Ignore mirroring control or above mirroring bit; instead provide four-screen VRAM
++++----- Lower nybble of mapper number
*/
#define  TRAINER_SZ         (512)
#define  have_trainer(m)    (m&(1<<2))

/* flags 7
76543210
||||||||
|||||||+- VS Unisystem
||||||+-- PlayChoice-10 (8KB of Hint Screen data stored after CHR data)
||||++--- If equal to 2, flags 8-15 are in NES 2.0 format
++++----- Upper nybble of mapper number
*/

extern struct ines_header *i_h;


extern int load_rom(char*);

#endif
