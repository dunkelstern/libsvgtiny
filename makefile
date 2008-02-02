#
# This file is part of Libsvgtiny
# Licensed under the MIT License,
#                http://opensource.org/licenses/mit-license.php
# Copyright 2008 James Bursa <james@semichrome.net>
#

SOURCE = svgtiny.c colors.c
HDRS = svgtiny.h

.PHONY: all install clean

CFLAGS = -std=c99 -g -W -Wall -Wundef -Wpointer-arith -Wcast-qual \
	-Wcast-align -Wwrite-strings -Wstrict-prototypes \
	-Wmissing-prototypes -Wmissing-declarations \
	-Wnested-externs -Winline -Wno-cast-align \
	`xml2-config --cflags`
LIBS = `xml2-config --libs`
ARFLAGS = cr

OBJS = $(SOURCE:.c=.o)

all: libsvgtiny.a svgtiny_test$(EXEEXT)

libsvgtiny.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

svgtiny_test$(EXEEXT): svgtiny_test.c libsvgtiny.a
	$(CC) $(CFLAGS) $(LIBS) -o $@ $^

clean:
	-rm *.o libsvgtiny.a svgtiny_test$(EXEEXT) colors.c

colors.c: colors.gperf
	gperf --output-file=$@ $<

.c.o: $(HDRS)
	$(CC) $(CFLAGS) -c -o $@ $<
