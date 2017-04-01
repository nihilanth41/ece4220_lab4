IDIR=../include
CC=/usr/local/opt/crosstool/arm-linux/gcc-3.3.4-glibc-2.3.2/bin/arm-linux-gcc
#CC=gcc 
CFLAGS=-I$(IDIR) -I../../includes/realtime/include -I../../includes/src/linux24/include 
CFLAGS+= -Wall -Wextra -Wshadow#-Werror -Wextra -Wshadow

ODIR=obj
LDLIBS =-lpthread -L../../includes/realtime/lib -L../../includes/realtime/src/linux24/lib

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


