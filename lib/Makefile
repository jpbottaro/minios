ODIR=../lib/obj
_OBJ=misc.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

CC=gcc
CFLAGS=-m32 -g -ggdb -Wall -Werror -O0 -I ../include\
  -fno-zero-initialized-in-bss -fno-stack-protector -ffreestanding

.PHONY=all clean

$(ODIR)/%.o: %.c
	@mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -c -o $@ $^

all: $(OBJ)
	@mkdir -p $(ODIR)
	@echo $(OBJ) > $(ODIR)/depend

clean:
	rm -rf $(ODIR)
