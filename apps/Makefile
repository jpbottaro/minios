ODIR=$(CURDIR)/obj
_OBJ=scall.o test.o
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
	$(LD) $(LDFLAGS) -o test obj/test.o obj/scall.o

clean:
	rm -rf $(ODIR)
	rm -f test
