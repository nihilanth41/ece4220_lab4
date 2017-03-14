TARGET := lab4_p2
#WARN := -W -Wall #-Wmissing-prototype -Wstrict-prototypes
#INCLUDE := -isystem /lib/modules/`uname -r`/build/include
INC_PATH := ../../../includes
INCLUDE :=  -I. -I${INC_PATH}/realtime/include -I${INC_PATH}/src/linux24/include
CFLAGS := -O0 -DMODULE -D__KERNEL__ ${WARN} ${INCLUDE} -g
CC	:= /usr/local/opt/crosstool/arm-linux/gcc-3.3.4-glibc-2.3.2/bin/arm-linux-gcc
${TARGET}.o: ${TARGET}.c

.PHONY: clean
clean:
	rm -rf ${TARGET}.o
