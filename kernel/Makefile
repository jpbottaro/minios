ODIR=../kernel/obj
_OBJ=kinit.o kernel.o gdt.o pic.o picarch.o idt.o vmm.o mm.o mmarch.o vga.o sched.o pm.o pmarch.o clock.o clockarch.o scall.o scallarch.o tss.o dev.o debug.o sem.o devfs.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

CC=gcc
CFLAGS=-m32 -g -ggdb -Wall -Werror -O0 -I ../include\
  -fno-zero-initialized-in-bss -fno-stack-protector -ffreestanding
NASM=nasm
NASMFLAGS=-felf

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
