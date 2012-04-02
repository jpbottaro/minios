DRIVER_NAME=ramdisk
DRIVER_DIR=drivers
ODIR=obj
_OBJ=ramdisk.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))
DEP=$(patsubst %,../$(DRIVER_DIR)/$(DRIVER_NAME)/$(ODIR)/%,$(_OBJ))

CC=gcc
CFLAGS=-m32 -g -ggdb -Wall -Werror -O0 -I ../../include\
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
	@echo $(DEP) > $(ODIR)/depend

clean:
	rm -rf $(ODIR)
