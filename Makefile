# -*- makefile -*-
#
# Copyright (c) 2004 Dan Villiom Podlaski Christiansen <danchr@daimi.au.dk>
# See the file LICENSE for details
#

SUBDIRS=framework kext util
SUBTARGETS=all clean dist install uninstall

$(SUBTARGETS):
	$(foreach i,$(SUBDIRS),$(MAKE) -C $i $@ &&) :

release:
	$(MAKE) RELEASE=yes clean dist
