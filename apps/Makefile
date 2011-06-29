ODIR=../apps/obj
OBJ=lineecho.pso echo.pso argc.pso ls.pso cat.pso cp.pso rm.pso cash.pso \
	mkdir.pso rmdir.pso mv.pso gp.pso init.pso pipes.pso

CC=gcc
CFLAGS=-m32 -Wall -Werror -O0 -I ../include -fno-zero-initialized-in-bss -fno-stack-protector -ffreestanding
NASM=nasm
NASMFLAGS=-felf32
LD=ld
LDFLAGS=-static -m elf_i386 --oformat binary -b elf32-i386 -e main -Ttext 0x400000

.PHONY=all clean

.SECONDARY:

$(ODIR)/%.o: %.c
	@mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -c -o $@ $^

$(ODIR)/%.o: %.asm
	@mkdir -p $(ODIR)
	$(NASM) $(NASMFLAGS) -o $@ $^

%.pso: $(ODIR)/pso_head.o $(ODIR)/%.o $(ODIR)/misc.o $(ODIR)/scall.o $(ODIR)/pso_tail.o
	$(LD) $(LDFLAGS) -o $@ $(ODIR)/pso_head.o  $(ODIR)/$(@:.pso=.o) $(ODIR)/misc.o $(ODIR)/scall.o $(ODIR)/pso_tail.o

all: $(OBJ)

clean:
	rm -rf $(ODIR)
	rm -f *.pso
