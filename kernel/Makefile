ODIR=../kernel/obj
_OBJ=kinit.o kernel.o gdt.o pic.o idt.o mm.o vga.o sched.o schedarch.o pm.o pmarch.o clock.o scall.o scallarch.o tss.o dev.o debug.o sem.o pipe.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

CC=gcc
CFLAGS=-m32 -g -ggdb -Wall -Werror -O0 -I ../include\
  -fno-zero-initialized-in-bss -fno-stack-protector -ffreestanding
NASM=nasm
NASMFLAGS=-felf32

.PHONY=all clean

$(ODIR)/%.o: %.c
	@mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -c -o $@ $^

$(ODIR)/%.o: %.asm
	@mkdir -p $(ODIR)
	$(NASM) $(NASMFLAGS) -o $@ $^

all: $(OBJ)
	@mkdir -p $(ODIR)
	@echo $(OBJ) > $(ODIR)/depend

clean:
	rm -rf $(ODIR)
