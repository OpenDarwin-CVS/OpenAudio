#*- makefile -*-
#
# Copyright (c) 2004, Dan Villiom Podlaski Christiansen
# See the file LICENSE for details
#

include config.mak

SUBDIRS=framework kext

SRCROOT=$(CURDIR)

all:
	$(foreach i,$(SUBDIRS),$(MAKE) -C $i all;)

clean:
	$(foreach i,$(SUBDIRS),$(MAKE) -C $i clean;)

dist:
	$(foreach i,$(SUBDIRS),$(MAKE) -C $i dist;)

install:
	$(foreach i,$(SUBDIRS),$(MAKE) -C $i install;)

release:
	sudo $(MAKE) clean dist RELEASE=yes


reload: unload load
