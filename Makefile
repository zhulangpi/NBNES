CC := gcc
LD := gcc
CFLAGS :=
LDFLAGS :=
INCLD_DIR := ./
LIBS := -lSDL2

OBJS := main.o load_rom.o cpu.o ppu.o sdl.o
all: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) -o a.out

%.o : %.c
	$(CC) $(CFLAGS) -I$(INCLD_DIR) $< -c -o $@
clean:
	rm a.out *.o
