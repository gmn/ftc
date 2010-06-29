##############################################################
# makefile 
##############################################################

EXE=ftc

DEBUG_CFLAGS=-g -Wall -D_DEBUG 
#note: some OPT_FLAGS may impede debugging
OPT_CFLAGS= -fomit-frame-pointer -funroll-loops -O2 -ffast-math -fexpensive-optimizations -falign-loops -falign-functions -falign-labels -fforce-addr -fforce-mem

LDFLAGS=-lm
LIBDIR=-L/usr/local/lib -L/usr/lib
INCDIR=-I/usr/include -I/usr/local/include

CFLAGS=-D_OSX 

CFLAGS+=${DEBUG_CFLAGS}
#CFLAGS+=$(OPT_CFLAGS)

CC=gcc

OBJ=ftc.o

.PHONY: all
all: ${EXE}

${EXE}: ${OBJ}
	${CC} $^ ${CFLAGS} -o $@ $(LDFLAGS) 

%.o: %.c
	${CC} ${CFLAGS} -c $^ $(INCDIR) $(LIBDIR)

clean:
	rm -f $(EXE) *.o *.log .gdb_history a.out 

install:
	install -c -m 755 ftc /usr/bin/
