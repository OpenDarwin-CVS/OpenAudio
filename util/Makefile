#*- makefile -*-
#
# Copyright (c) 2004, Dan Villiom Podlaski Christiansen
# See the file LICENSE for details
#

SRCROOT=$(CURDIR)/..
BINS=str2plist bswap selout setvol mute

include $(SRCROOT)/config.mak

all: $(BINS)

dist: $(BINS:%=$(SRCROOT)/%)

clean:
	$(RM) $(BINS) $(BINS:%=%.o) $(BINS:%=$(SRCROOT)/%)

install: $(BINS:%=$(EPREFIX)/%)

$(EPREFIX)/%: $(SRCROOT)/%
	$(INSTALL) $< $(EPREFIX)

$(SRCROOT)/%: %
	$(INSTALL) $< $(SRCROOT)

.depend: $(BINS:%=%.c) Makefile
	$(RM) $@
	$(CC) -M $(CPPFLAGS) $(BINS:%=%.c) > $@

-include .depend

%: %.o
	$(CC) $(LDFLAGS) $< -o $@
