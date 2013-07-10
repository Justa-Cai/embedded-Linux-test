# CROSS COMPILE 
CROSS:=arm-linux-

# COMPILE TOOLS
CC:=$(CROSS)gcc
CXX:=$(CROSS)g++
LD:=$(CROSS)ld
AR:=$(CROSS)ar
STRIP:=$(CROSS)strip

# FLAGS
CFLAGS:=-march=armv4t

CPPFLAGS:=$(CFLAGS)

LDFLAGS:=-march=armv4t

# SOURCE 
SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))

# TARGET 
EXE:=$(shell basename $(shell pwd))

INSTALL_PATH:=/tftproot/mini2440/rootfs/
all:$(EXE)

$(EXE):$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^
	$(STRIP) $@
	@cp -fv $@ $(INSTALL_PATH)
	
