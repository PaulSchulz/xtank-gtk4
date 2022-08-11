.PHONY: all clean

PROG=  xtank-gtk4

CFLAGS= -I${CURDIR} -I${CURDIR}/include \
	-O0 -g

CFLAGS+= -DX11 -DUNIX -DS1024x864 -DKEYPAD_DETECT -DSTINGY_REDRAW \
	-DBATCH_LINES -DBATCH_POINTS -DBATCH_COLOR_LINES \
	-DCODE_SIZE=0x20000 -DSTACK_SIZE=0x8000 \
	-DNEW_LIMITS -DSLOWER

CFLAGS+= CFLAGS0+= -DDYNALOAD -DMULTI_SYNC -DTHREAD_SWAPCONTEXT

CFLAGS+= -DXTANK_DIR="\"${CURDIR}/..\""

SRCS=  main.c \
       escher.c \
	   version.c

OBJS:=	$(patsubst %.c,%.o,$(SRCS))

CFLAGS0= -Iinclude -DUNIX -DX11

DEFINES= -DALLDEFINES="\"${CFLAGS0}\""

CFLAGS= `pkg-config --cflags gtk4` ${CFLAGS0} ${DEFINES}
LDLIBS= -lm  `pkg-config --libs gtk4`
CC = gcc

all: ${PROG}

##############################################################################
${PROG}: ${OBJS}
	gcc  $^ ${LOADLIBES} ${LDLIBS} -o $@

#xtank: xtank.o xtank.gresources.o
#	gcc -o $@ $@.o $@.gresources.o `pkg-config --libs gtk4` -lm

# xtank.o: xtank.c
#	gcc -c -o $@  `pkg-config --cflags gtk4` $<

xtank.gresources.o: xtank.gresources.c
	gcc -c -o $@ `pkg-config --cflags gtk4` $<

xtank.gresources.c: xtank.gresource.xml xtank.ui
	glib-compile-resources $< --target=$@ --generate-source

##############################################################################
# Utilities
version.c: ../bin/newversion.sh .buildnumber
	echo "${.CURDIR}"
	echo "${CURDIR}"
	sh ../bin/newversion.sh

clean:
	-rm *.o *.gresources.c

dist-clean:
	-rm ${PROG}
