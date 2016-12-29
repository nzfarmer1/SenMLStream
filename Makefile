# try to do some autodetecting
UNAME := $(shell uname -s)

ifeq "$(UNAME)" "Darwin"
	OS=macosx
endif
ifeq "$(OS)" "Windows_NT"
	OS=windows
endif
ifeq "$(UNAME)" "Linux"
	OS=linux
endif

ifeq "$(OS)" "linux"

CC=g++
CFLAGS += -DLINUX 

endif
#################  Mac OS X  ##################################################
ifeq "$(OS)" "macosx"

EXE_SUFFIX=

CFLAGS+= $(ARCHS) -DMACOSX #  -DSERIALPORTDEBUG
CFLAGS += -mmacosx-version-min=10.6
LDFLAGS = -I../arduino-serial -L../arduino-serial
CFLAGS_MONGOOSE=  -I./mongoose -pthread -g 
CC=g++

endif

#################  Windows  ##################################################
ifeq "$(OS)" "windows"

EXE_SUFFIX=.exe

CFLAGS_MONGOOSE = -I./mongoose -mthreads

endif


#################  Common  ##################################################

CFLAGS += $(INCLUDES) -O -Wall -g -std=c++0x -D_SIMULATOR


all: test


senmlstream: senmlstream.h
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o senmlstream.o senmlstream.c

FixedQueue: FixedQueue.h
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o FixedQueue.o FixedQueue.h
	

test:   test/test.c senmlstream cmp.o  FixedQueue buffered-serial.h 
	$(CC) $(CFLAGS) $(LDFLAGS) test/test.c -o test/test$(EXE_SUFFIX) buffered-serial.h  ../arduino-serial/arduino-serial-lib.o  senmlstream.o cmp.o 


.c.o:
	$(CC) $(CFLAGS) -c $*.c -o $*.o


clean:
	rm -f $(OBJ)  *.o *.a
	rm -f linux-serial test/test

