#
# This file is part of Libsvgtiny
# Licensed under the MIT License,
#                http://opensource.org/licenses/mit-license.php
# Copyright 2008 James Bursa <james@semichrome.net>
#

SOURCE = svgtiny.c colors.c
HDRS = svgtiny.h

CFLAGS = -std=c99 -W -Wall -Wundef -Wpointer-arith -Wcast-qual \
	-Wcast-align -Wwrite-strings -Wstrict-prototypes \
	-Wmissing-prototypes -Wmissing-declarations \
	-Wnested-externs -Winline -Wno-cast-align
ARFLAGS = cr

ifeq ($(TARGET),riscos)
GCCSDK_INSTALL_CROSSBIN ?= /home/riscos/cross/bin
GCCSDK_INSTALL_ENV ?= /home/riscos/env
CC = $(GCCSDK_INSTALL_CROSSBIN)/gcc
AR = $(GCCSDK_INSTALL_CROSSBIN)/ar
CFLAGS += -Driscos -mpoke-function-name -I$(GCCSDK_INSTALL_ENV)/include \
	-I$(GCCSDK_INSTALL_ENV)/include/libxml2
LIBS = -L$(GCCSDK_INSTALL_ENV)/lib -lxml2 -lz
EXEEXT = ,ff8
else
CFLAGS += -g `xml2-config --cflags` -fgnu89-inline
LIBS = `xml2-config --libs`
endif

ifeq ($(TARGET),)
OBJDIR = objects
LIBDIR = lib
BINDIR = bin
else
OBJDIR = $(TARGET)-objects
LIBDIR = $(TARGET)-lib
BINDIR = $(TARGET)-bin
endif

OBJS = $(addprefix $(OBJDIR)/, $(SOURCE:.c=.o))

.PHONY: all install clean

all: $(LIBDIR)/libsvgtiny.a $(BINDIR)/svgtiny_test$(EXEEXT) colors.c

$(LIBDIR)/libsvgtiny.a: $(OBJS)
	@echo "    LINK:" $@
	@mkdir -p $(LIBDIR)
	@$(AR) $(ARFLAGS) $@ $(OBJS)

$(BINDIR)/svgtiny_test$(EXEEXT): svgtiny_test.c $(LIBDIR)/libsvgtiny.a
	@echo "    LINK:" $@
	@mkdir -p $(BINDIR)
	@$(CC) $(CFLAGS) $(LIBS) -o $@ $^

$(OBJDIR)/%.o: %.c $(HDRS)
	@echo " COMPILE:" $<
	@mkdir -p $(OBJDIR)
	@$(CC) $(CFLAGS) -c -o $@ $<

%.c: %.gperf
	@echo "   GPERF:" $<
	@gperf --output-file=$@ $<

clean:
	-rm $(OBJS) $(LIBDIR)/libsvgtiny.a $(BINDIR)/svgtiny_test$(EXEEXT) colors.c

