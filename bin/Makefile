_DEPEND=kernel fs lib
DEPEND=$(patsubst %,../%/obj/depend,$(_DEPEND))

# programs
MAKE=make
LD=ld
MCOPY=mcopy
MKFS=/sbin/mkfs.vfat

# program flags
NASMFLAGS=-felf32
LDFLAGS=-static -m elf_i386 --oformat binary -b elf32-i386 -e start \
		-Ttext 0x1200
CFLAGS=-m32 -g -ggdb -Wall -Werror -O0 -I ../include\
  -fno-zero-initialized-in-bss -fno-stack-protector -ffreestanding

# floppy image
DISK_LABEL="PSO"
IMG_BASE=base.img
IMG_FLOPPY=floppy.img
BIN_BOOT=../boot/floppyboot.bin

# misc
BIN_KERN=kernel.bin
FSIMAGE=fsimage
KERNELMEM=126464	# 0x20000 - 0x1200

.PHONY=clean all image

all: $(IMG_FLOPPY)

test:
	make clean && make && bochs

$(ODIR)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(ODIR)/%.o: $(SRC)/%.asm
	$(NASM) $(NASMFLAGS) -o $@ $^

$(IMG_FLOPPY): $(BIN_BOOT) $(BIN_KERN)
	@dd if=/dev/zero of=$@ bs=512 count=2880 2>/dev/null
	$(MKFS) -F 12 -f 2 -n $(DISK_LABEL) $@
	dd if=$< of=$@ bs=1 count=3 conv=notrunc 2>/dev/null
	dd if=$< of=$@ bs=1 count=450 seek=62 skip=62 conv=notrunc 2>/dev/null
	mcopy -obi $@ $(BIN_KERN) ::kernel.bin
	for T in $(TASKS); do mcopy -obi $@ $$T ::`basename $$T`; done;

$(BIN_KERN):
	@echo Building kernel
	@$(MAKE) -C ../kernel
	@echo Building file system
	@$(MAKE) -C ../fs
	@echo Building lib
	@$(MAKE) -C ../lib
	@echo Linking $@
	@cat $(DEPEND) 2>/dev/null | xargs $(LD) $(LDFLAGS) -o $@
	@echo Adding fsimage to $@
	@mv $@ $@.tmp
	@dd if=/dev/zero bs=1 count=$(KERNELMEM) of=$@ status=noxfer \
															>/dev/null 2>&1
	@dd if=$@.tmp bs=1 of=$@ conv=notrunc count=$(KERNELMEM) \
											  status=noxfer >/dev/null 2>&1
	@dd if=$(FSIMAGE) bs=1 of=$@ conv=notrunc seek=$(KERNELMEM) \
											  status=noxfer >/dev/null 2>&1
	@rm $@.tmp

clean:
	@$(MAKE) --no-print-directory -C ../kernel clean
	@$(MAKE) --no-print-directory -C ../fs clean
	@$(MAKE) --no-print-directory -C ../lib clean
	rm -f $(BIN_KERN)
	rm -f $(IMG_FLOPPY)
