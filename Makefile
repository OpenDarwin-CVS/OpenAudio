
include config.mak

SUBDIRS=kext
SUBTARGETS=all dist clean

SRCROOT=$(CURDIR)

all: $(SUBDIRS)(all)
dist: $(SUBDIRS)(dist)
clean: $(SUBDIRS)(clean)
installhdrs: $(SUBDIRS)(installhdrs)

$(SUBDIRS)($(SUBTARGETS)):
	$(MAKE) -C $@ $%

clean: