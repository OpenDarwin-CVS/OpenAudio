#*- makefile -*-
#
# Copyright (c) 2004, Dan Villiom Podlaski Christiansen
# See the file LICENSE for details
#

include config.mak

SUBDIRS=framework kext util
SUBTARGETS=all clean dist install

SRCROOT=$(CURDIR)

$(SUBTARGETS):
	$(foreach i,$(SUBDIRS),$(MAKE) -C $i $@ &&) :

release:
	$(MAKE) RELEASE=yes clean dist
