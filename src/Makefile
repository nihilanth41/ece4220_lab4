IDIR=../include
CC=/usr/local/opt/crosstool/arm-linux/gcc-3.3.4-glibc-2.3.2/bin/arm-linux-gcc
CFLAGS=-I$(IDIR) -Wall -Werror -Wmissing-prototypes -g

ODIR=obj
LDLIBS =-lpthread

_DEPS = serial_ece4220.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = lab4_main.o serial_ece4220.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c 
	$(CC) -c -o $@ $< $(CFLAGS)

lab4_main: $(OBJ)
	$(CC) -o $@.bin $^ $(CFLAGS) $(LDLIBS)

.PHONY: clean

clean:
	rm -rf $(ODIR)/*.o *~ core $(INCDIR)/*~


