
include config.mak

SUBDIRS=kext
SUBTARGETS=all dist clean

export SRCROOT=$(CURDIR)
export DSTROOT=$(SRCROOT)

all: $(SUBDIRS)(all)
dist: $(SUBDIRS)(dist)
clean: $(SUBDIRS)(clean)
installhdrs: $(SUBDIRS)(installhdrs)

$(SUBDIRS)($(SUBTARGETS)):
	$(MAKE) -C $@ $%

clean: