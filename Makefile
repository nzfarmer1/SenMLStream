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

CFLAGS+= $(ARCHS) -DMACOSX  -D_SIMULATOR -DXDEBUG # -DSERIALPORTDEBUG 
CFLAGS += -g -mmacosx-version-min=10.6 -DSMLDEBUG_LINUX -D__SMLDEBUG_LINUX
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

SRCS = $(wildcard test/*.c)
LIBS = $(wildcard *.c)

PROGS = $(patsubst %.c,%,$(SRCS))
MLIBS = $(patsubst X.c,X,$(LIBS))

OBJS = ../arduino-serial/arduino-serial-lib.o  senmlstream.o cmp.o 

all: $(MLIBS) $(PROGS)

arduino-serial-lib:
	cd ../arduino-serial && make SERIALPORTDEBUG=0x1

senmlstream: senmlstream.h
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o senmlstream.o senmlstream.cpp

FixedQueue: FixedQueue.h
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o FixedQueue.o FixedQueue.h


cmp: cmp.c cmp.h
	$(CC) $(CFLAGS) -c $(@).c -o $(@).o


%: %.c senmlstream  FixedQueue cmp arduino-serial-lib
	$(CC) $(CFLAGS) $(LDFLAGS)  $(@).c -o $(@) $(OBJS)


clean:
	cd ../arduino-serial && make clean
	rm -f  *.o *.a
	rm -f $(PROGS)
