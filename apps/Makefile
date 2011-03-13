ODIR=$(CURDIR)/obj
_OBJ=misc.o scall.o lineecho.o echo.o argc.o ls.o cat.o cash.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

CC=gcc
CFLAGS=-m32 -Wall -Werror -O0 -I ../include\
  -fno-zero-initialized-in-bss -fno-stack-protector -ffreestanding
NASM=nasm
NASMFLAGS=-felf32
LD=ld
LDFLAGS=-static -m elf_i386 --oformat binary -b elf32-i386 -e main \
		-Ttext 0x400000

.PHONY=all clean

$(ODIR)/%.o: %.c
	@mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -c -o $@ $^

$(ODIR)/%.o: %.asm
	@mkdir -p $(ODIR)
	$(NASM) $(NASMFLAGS) -o $@ $^

all: $(OBJ)
	$(LD) $(LDFLAGS) -o lineecho obj/lineecho.o obj/scall.o obj/misc.o
	$(LD) $(LDFLAGS) -o cash obj/cash.o obj/scall.o obj/misc.o
	$(LD) $(LDFLAGS) -o echo obj/echo.o obj/scall.o obj/misc.o
	$(LD) $(LDFLAGS) -o argc obj/argc.o obj/scall.o obj/misc.o
	$(LD) $(LDFLAGS) -o ls obj/ls.o obj/scall.o obj/misc.o
	$(LD) $(LDFLAGS) -o cat obj/cat.o obj/scall.o obj/misc.o

clean:
	rm -rf $(ODIR)
	rm -f lineecho cash echo argc ls cat
