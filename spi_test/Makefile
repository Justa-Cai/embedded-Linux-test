CROSS:=arm-none-linux-gnueabi-
CC:=$(CROSS)gcc
CPP:=$(CROSS)g++

#CFLAGS+=-march=armv4t
#LDFLAGS+=-march=armv4t

CFLAGS+= -g

CPPFLAGS:=$(CFLAGS)

EXE:=$(shell basename $(shell pwd))

SCRS:=$(wildcard *.c)
OBJS:=$(patsubst %.c,%.o,$(SCRS))
INSTALL_PATH?=/tftproot/tiny210/rootfs/
all:$(EXE)

$(EXE):$(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@cp -av $@ $(INSTALL_PATH)
	
clean:
	@rm -rf $(OBJS) $(EXE)
