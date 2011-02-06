_DEPEND=kernel fs lib
DEPEND=$(patsubst %,../%/obj/depend,$(_DEPEND))

DISKIMG=diskette.img
FSIMAGE=fsimage
KERNELMEM=126464	# 0x20000 - 0x1200

MAKE=make
LD=ld
LDTASKFLAGS=-static -m elf_i386 --oformat binary -b elf32-i386 -e start
LDFLAGS=-static -m elf_i386 --oformat binary -b elf32-i386 -e start \
		-Ttext 0x1200
MCOPY=mcopy

.PHONY=clean all image

all: image

$(ODIR)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(ODIR)/%.o: $(SRC)/%.asm
	$(NASM) $(NASMFLAGS) -o $@ $^

kernel.bin:
	@echo Building kernel
	@$(MAKE) -C ../kernel
	@echo Building file system
	@$(MAKE) -C ../fs
	@echo Building lib
	@$(MAKE) -C ../lib
	@echo Linking kernel.bin
	@cat $(DEPEND) 2>/dev/null | xargs $(LD) $(LDFLAGS) -o $@ $^
	@echo Adding fsimage to kernel.bin
	@mv kernel.bin kernel.bin.tmp
	@dd if=/dev/zero bs=1 count=$(KERNELMEM) of=kernel.bin status=noxfer \
															>/dev/null 2>&1
	@dd if=kernel.bin.tmp bs=1 of=kernel.bin conv=notrunc count=$(KERNELMEM) \
											  status=noxfer >/dev/null 2>&1
	@dd if=$(FSIMAGE) bs=1 of=kernel.bin conv=notrunc seek=$(KERNELMEM) \
											  status=noxfer >/dev/null 2>&1
	@rm kernel.bin.tmp

image: kernel.bin
	@echo 'Copying kernel.bin in the diskette image'
	@$(MCOPY) -o -i $(DISKIMG) kernel.bin ::/ 

clean:
	@$(MAKE) --no-print-directory -C ../kernel clean
	@$(MAKE) --no-print-directory -C ../fs clean
	@$(MAKE) --no-print-directory -C ../lib clean
	rm -f kernel.bin
