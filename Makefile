# -*- makefile -*-
#
# Copyright (c) 2004 Dan Villiom Podlaski Christiansen <danchr@opendarwin.org>
# See the file LICENSE for details
#

SUBDIRS=framework kext util
SUBTARGETS=all clean install uninstall

$(SUBTARGETS):
	$(foreach i,$(SUBDIRS),$(MAKE) $(MFLAGS) -C $i $@ &&) :

release: MFLAGS+=RELEASE=yes
release: clean all
