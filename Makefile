
include config.mak

SUBDIRS=kext

SRCROOT=$(CURDIR)

all:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i all; done

clean:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i clean; done

dist:
	@for i in $(SUBDIRS); do $(MAKE) -C $$i dist; done

release:
	sudo $(MAKE) clean dist RELEASE=

install:
	$(INSTALL) $(KEXT) $(KPREFIX)
	$(INSTALL) $(FRAMEWORK) $(FPREFIX)


load: dist
	kextload -t -s $(KEXT)/Contents/Resources $(KEXT)

unload: dist
	kextunload $(KEXT)

reload: unload load
