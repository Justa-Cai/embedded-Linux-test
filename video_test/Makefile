CROSS_COMPILER= /toolchain/arm-2012.03/bin/arm-none-linux-gnueabi-

CC := $(CROSS_COMPILER)gcc
CXX := $(CROSS_COMPILER)g++
LD := $(CROSS_COMPILER)ld
AR := $(CROSS_COMPILER)ar
STRIP := $(CROSS_COMPILER)strip

LDFLAGS += -mtune=arm920t -march=armv4t -mabi=aapcs-linux -msoft-float
CFLAGS += -I./inc
CFLAGS += -Wall -std=c99
CFLAGS +=  -mtune=arm920t -march=armv4t -mabi=aapcs-linux -msoft-float -lpthread
CPPFLAGS = $(CFLAGS)


EXE:=$(shell basename $(shell pwd))
EXE2:=fb_capture
EXE3:=v4l_test
SRCS=main.c
OBJS=$(patsubst %.c,%.o,$(SRCS))

EXE:=$(shell basename $(shell pwd))

INSTALL_PATH:=/tftproot/mini2440/rootfs/

all: $(EXE) $(EXE2) $(EXE3)

$(EXE3): v4l_test.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -L./sdk/lib/ -lv4l2 -l v4lconvert
	@$(STRIP) $@	
	@cp -fv $@ $(INSTALL_PATH)

$(EXE2): fb_capture.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@$(STRIP) $@	
	@cp -fv $@ $(INSTALL_PATH)

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@$(STRIP) $@	
	@cp -fv $(EXE) $(INSTALL_PATH)
	
install:
	@cp -fv $(EXE) $(INSTALL_PATH)
clean:
	rm -f $(OBJS) $(EXE) *.bz2
	
