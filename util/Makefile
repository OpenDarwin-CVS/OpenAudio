#*- makefile -*-
#
# Copyright (c) 2004, Dan Villiom Podlaski Christiansen
# See the file LICENSE for details
#

SRCROOT=$(CURDIR)/..
BINS=str2plist bswap selout

include ../config.mak

all: $(BINS)

dist:

clean:
	$(RM) $(BINS) $(BINS:=.o)

install:
	$(INSTALL) -d
	$(INSTALL) $(BINS) $(EPREFIX)

%: %.o
	$(CC) $(LDFLAGS) $< -o $@
