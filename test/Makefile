.PHONY: all clean

PROG=  xtank-test

CFLAGS0= -I${CURDIR} -I${CURDIR}/../src/include \
	-O0 -g

# CFLAGS0+= -DX11 -DUNIX -DS1024x864 -DKEYPAD_DETECT -DSTINGY_REDRAW \
# 	-DBATCH_LINES -DBATCH_POINTS -DBATCH_COLOR_LINES \
#	-DCODE_SIZE=0x20000 -DSTACK_SIZE=0x8000 \
#	-DNEW_LIMITS -DSLOWER

CFLAGS0+= -DUNIX

# CFLAGS0+= -DDYNALOAD -DMULTI_SYNC -DTHREAD_SWAPCONTEXT

# CFLAGS1+= -DXTANK_DIR="\"${CURDIR}/..\""

# DEFINES= -DALLDEFINES="\"${CFLAGS0}\""

CFLAGS+= ${DEFINES} ${CFLAGS0} ${CFLAGS1}

SRCS=  main.c

OBJS:=	$(patsubst %.c,%.o,$(SRCS))

CFLAGS+= `pkg-config --cflags gtk4`
LDLIBS= -lm  `pkg-config --libs gtk4`
CC = gcc

all: ${PROG}

##############################################################################
${PROG}: ${OBJS}
	gcc  ${CFLAGS} $^ ${LOADLIBES} ${LDLIBS} -o $@

#xtank: xtank.o xtank.gresources.o
#	gcc -o $@ $@.o $@.gresources.o `pkg-config --libs gtk4` -lm

# xtank.o: xtank.c
#	gcc -c -o $@  `pkg-config --cflags gtk4` $<

# xtank.gresources.o: xtank.gresources.c
#	gcc -c -o $@ `pkg-config --cflags gtk4` $<

# xtank.gresources.c: xtank.gresource.xml xtank.ui
#	glib-compile-resources $< --target=$@ --generate-source

##############################################################################
# Utilities
version.c: ../bin/newversion.sh .buildnumber
	echo "${.CURDIR}"
	echo "${CURDIR}"
	sh ../bin/newversion.sh

clean:
#	-rm *.o *.gresources.c

dist-clean:
	-rm ${PROG}
