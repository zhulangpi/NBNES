CC := gcc
LD := gcc
CFLAGS :=
LDFLAGS :=
INCLD_DIR := ./

OBJS := main.o load_rom.o cpu.o
all: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o a.out

%.o : %.c
	$(CC) $(CFLAGS) -I$(INCLD_DIR) $< -c -o $@
clean:
	rm a.out *.o
