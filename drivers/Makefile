ODIR=obj

_DEPEND=tty serial hdd ramdisk
DEPEND=$(patsubst %,%/obj/depend,$(_DEPEND))

# programs
MAKE=make

.PHONY=clean all

all:
	@$(MAKE) -C tty
	@$(MAKE) -C serial
	@$(MAKE) -C hdd
	@$(MAKE) -C ramdisk
	@mkdir -p $(ODIR)
	@cat $(DEPEND) > $(ODIR)/depend

clean:
	@$(MAKE) -C tty clean
	@$(MAKE) -C serial clean
	@$(MAKE) -C hdd clean
	@$(MAKE) -C ramdisk clean
	rm -rf $(ODIR)
