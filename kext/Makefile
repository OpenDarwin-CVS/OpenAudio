# -*- makefile -*-
#
# Copyright (c) 2004 Dan Villiom Podlaski Christiansen <danchr@opendarwin.org>
# See the file LICENSE for details
#

SRCROOT=$(CURDIR)/..

TARGET=$(KEXT)
OBJS=module.o ODBSDClient.o ODAudioBSDClient.o
SRCS=$(shell for file in $(OBJS); do for ext in .cc .cpp .mm .c .m; do \
		test -e $${file%.o}$$ext && echo $${file%%.o}$$ext; \
	done; done)
KERNEL=yes

include $(SRCROOT)/config.mak

all: $(PRODUCT)

clean:
	$(RM) $(OBJS) $(PRODUCT) .depend

install: $(KPREFIX)/$(notdir $(TARGET))

uninstall:
	$(RM) $(KPREFIX)/$(notdir $(TARGET))

reload: $(KPREFIX)/$(notdir $(TARGET))
	kextunload $(KPREFIX)/$(notdir $(TARGET))
	$(RM) /System/Library/{Extensions.mkext,Extensions.kextcache}
	kextload $(KPREFIX)/$(notdir $(TARGET))

$(PRODUCT): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(KPREFIX)/$(notdir $(TARGET)): $(PRODUCT) Info.plist
	$(RM) $@
	$(INSTALL) -d $@/Contents/MacOS
	$(INSTALL) -m 644 Info.plist $@/Contents
	$(INSTALL) -m 644 $(PRODUCT) $@/Contents/MacOS

.depend: $(SRCS) Makefile $(SRCROOT)/config.mak
	$(RM) $@
	$(CC) -M $(CPPFLAGS) $(SRCS) > $@

-include .depend
