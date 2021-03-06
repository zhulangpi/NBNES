CC := gcc
LD := gcc
CFLAGS := -fsigned-char
LDFLAGS :=
INCLD_DIR := ./

OBJS := main.o load_rom.o cpu.o ppu.o display.o input.o
all: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o a.out

%.o : %.c
	$(CC) $(CFLAGS) -I$(INCLD_DIR) $< -c -o $@
clean:
	rm a.out *.o
