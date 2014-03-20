_DEPEND=kernel fs lib drivers
DEPEND=$(patsubst %,../%/obj/depend,$(_DEPEND))

# programs
MAKE=make
LD=ld
MCOPY=mcopy
MKFS=/sbin/mkfs.vfat

# program flags
NASMFLAGS=-felf
LDFLAGS=-static -m elf_i386 -g -b elf32-i386 -e start \
		-Ttext 0x1200
CFLAGS=-m32 -g -ggdb -Wall -Werror -O0 -I ../include\
  -fno-zero-initialized-in-bss -fno-stack-protector -ffreestanding

# floppy image
DISK_LABEL="MINIOS"
IMG_BASE=base.img
IMG_FLOPPY=floppy.img
BIN_BOOT=../boot/floppyboot.bin

# misc
KERNEL=kernel
KERNEL_SYM=kernel.sym

.PHONY=clean all image

all: $(IMG_FLOPPY)

test:
	make clean && make && bochs -q

$(ODIR)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(ODIR)/%.o: $(SRC)/%.asm
	$(NASM) $(NASMFLAGS) -o $@ $^

$(IMG_FLOPPY): $(BIN_BOOT) $(KERNEL)
	@echo Building floppy image
	@dd if=/dev/zero of=$@ bs=512 count=2880 2>/dev/null
	@$(MKFS) -F 12 -f 2 -n $(DISK_LABEL) $@ >/dev/null
	@dd if=$< of=$@ bs=1 count=3 conv=notrunc 2>/dev/null
	@dd if=$< of=$@ bs=1 count=450 seek=62 skip=62 conv=notrunc 2>/dev/null
	@mcopy -obi $@ $(KERNEL) ::kernel.bin

$(KERNEL)::
	@echo Building kernel
	@$(MAKE) -C ../kernel
	@echo Building file system
	@$(MAKE) -C ../fs
	@echo Building lib
	@$(MAKE) -C ../lib
	@echo Building drivers
	@$(MAKE) -C ../drivers
	@echo Linking $@
	@cat $(DEPEND) 2>/dev/null | xargs $(LD) $(LDFLAGS) -o $@
	@echo Extracting symbols
	@nm -n $(KERNEL) | grep -v '\( [aUw] \)\|\(__crc_\)\|\( \$$[adt]\)' | awk '{print $$1, $$3}' > $(KERNEL_SYM)
	@objcopy -O binary kernel



clean:
	@$(MAKE) --no-print-directory -C ../kernel clean
	@$(MAKE) --no-print-directory -C ../fs clean
	@$(MAKE) --no-print-directory -C ../lib clean
	@$(MAKE) -C ../drivers clean
	rm -f $(KERNEL)
	rm -f $(IMG_FLOPPY)
