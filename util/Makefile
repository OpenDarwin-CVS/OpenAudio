# -*- makefile -*-
#
# Copyright (c) 2004 Dan Villiom Podlaski Christiansen <danchr@daimi.au.dk>
# See the file LICENSE for details
#

SRCROOT=$(CURDIR)/..
BINS=str2plist bswap selout setvol mute mix

include $(SRCROOT)/config.mak

all: $(BINS)

dist: $(BINS:%=$(SRCROOT)/%)

clean:
	$(RM) $(BINS) $(BINS:%=%.o) $(BINS:%=$(SRCROOT)/%)

install: $(BINS:%=$(EPREFIX)/%)

uninstall:
	$(RM) $(BINS:%=$(EPREFIX)/%)

$(EPREFIX)/%: $(SRCROOT)/%
	$(INSTALL) $< $(EPREFIX)

$(SRCROOT)/%: %
	$(INSTALL) $< $(SRCROOT)

.depend: $(BINS:%=%.c) Makefile
	$(RM) $@
	$(CC) -M $(CPPFLAGS) $(BINS:%=%.c) > $@

-include .depend
