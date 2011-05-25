ODIR=obj

_DEPEND=tty serial hdd
DEPEND=$(patsubst %,%/obj/depend,$(_DEPEND))

# programs
MAKE=make

.PHONY=clean all

all:
	@$(MAKE) -C tty
	@$(MAKE) -C serial
	@$(MAKE) -C hdd
	@mkdir -p $(ODIR)
	@cat $(DEPEND) > $(ODIR)/depend

clean:
	@$(MAKE) -C tty clean
	@$(MAKE) -C serial clean
	@$(MAKE) -C hdd clean
	rm -rf $(ODIR)
