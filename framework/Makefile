# -*- makefile -*-
#
# Copyright (c) 2004 Dan Villiom Podlaski Christiansen <danchr@opendarwin.org>
# See the file LICENSE for details
#

SRCROOT=$(CURDIR)/..

TARGET=$(FRAMEWORK)
OBJS=framework.o
SRCS=$(shell for file in $(OBJS); do for ext in .cc .cpp .mm .c .m; do \
		test -e $${file%.o}$$ext && echo $${file%%.o}$$ext; \
	done; done)

include $(SRCROOT)/config.mak

all: $(PRODUCT)

install: $(FPREFIX)/$(notdir $(TARGET))

uninstall:
	$(RM) $(FPREFIX)/$(notdir $(TARGET))

clean:
	$(RM) $(OBJS) $(PRODUCT) .depend

$(PRODUCT): $(OBJS)
	$(CC) -dynamiclib $(OBJS) $(LDFLAGS) -o $(PRODUCT)

$(FPREFIX)/$(notdir $(TARGET)): $(PRODUCT)
	$(RM) $@
	$(INSTALL) -d $@/Headers $@/Resource
	$(INSTALL) $(PRODUCT) $@
	$(INSTALL) $(wildcard $(SRCROOT)/include/*.h) $@/Headers

.depend: $(SRCS) Makefile $(SRCROOT)/config.mak
	$(RM) $@
	$(CC) -M $(CPPFLAGS) $(SRCS) > $@

-include .depend
