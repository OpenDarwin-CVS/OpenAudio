# -*- makefile -*-
#
# Copyright (c) 2004 Dan Villiom Podlaski Christiansen <danchr@opendarwin.org>
# See the file LICENSE for details
#

SRCROOT=$(CURDIR)/..
BINS=str2plist bswap selout setvol mute mix

include $(SRCROOT)/config.mak

all: $(BINS)

clean:
	$(RM) $(BINS) $(BINS:%=%.o)

install: $(BINS:%=$(EPREFIX)/%)

uninstall:
	$(RM) $(BINS:%=$(EPREFIX)/%)

$(EPREFIX)/%: %
	$(INSTALL) $< $(EPREFIX)

.depend: $(BINS:%=%.c) Makefile $(SRCROOT)/config.mak
	$(RM) $@
	$(CC) -M $(CPPFLAGS) $(BINS:%=%.c) > $@

-include .depend
