ODIR=obj

_DEPEND=tty floppy
DEPEND=$(patsubst %,%/obj/depend,$(_DEPEND))

# programs
MAKE=make

.PHONY=clean all

all:
	@$(MAKE) -C tty
	@$(MAKE) -C floppy
	@mkdir -p $(ODIR)
	@cat $(DEPEND) > $(ODIR)/depend

clean:
	@$(MAKE) -C tty clean
	@$(MAKE) -C floppy clean
	rm -rf $(ODIR)
