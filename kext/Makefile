#*- makefile -*-
#
# Copyright (c) 2004, Dan Villiom Podlaski Christiansen
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

dist: $(TARGET)

clean:
	$(RM) $(OBJS) $(PRODUCT) $(TARGET) .depend

install: $(KPREFIX)/$(notdir $(TARGET))

.depend: $(SRCS) Makefile
	$(RM) $@
	$(CC) -M $(CPPFLAGS) $(SRCS) > $@

-include .depend

$(PRODUCT): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(TARGET): $(PRODUCT)
	$(RM) $@
	$(INSTALL) -d $@/Contents/MacOS $@/Contents/Resources
	$(INSTALL) -m 644 Info.plist $@/Contents
	$(INSTALL) -m 644 $(PRODUCT) $@/Contents/MacOS

$(KPREFIX)/$(notdir $(TARGET)): $(TARGET)
	$(RM) $(KPREFIX)/$(notdir $(TARGET))
	$(CP) $(TARGET) $(KPREFIX)

reload: $(KPREFIX)/$(notdir $(TARGET))
	kextunload $(KPREFIX)/$(notdir $(TARGET))
	kextload $(KPREFIX)/$(notdir $(TARGET))