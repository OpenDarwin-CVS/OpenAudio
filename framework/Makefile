#*- makefile -*-
#
# Copyright (c) 2004, Dan Villiom Podlaski Christiansen
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

dist: $(TARGET)

install: $(FPREFIX)/$(notdir $(TARGET))

uninstall:
	$(RM) $(FPREFIX)/$(notdir $(TARGET))

clean:
	$(RM) $(OBJS) $(PRODUCT) $(TARGET) .depend

.depend: $(SRCS) Makefile
	$(RM) $@
	$(CC) -M $(CPPFLAGS) $(SRCS) > $@

$(TARGET): $(PRODUCT)
	$(RM) $(TARGET)
	$(INSTALL) -d $(TARGET)/Headers $(TARGET)/Resource
	$(INSTALL) $(PRODUCT) $(TARGET)
	$(INSTALL) $(wildcard $(SRCROOT)/include/*.h) $(TARGET)/Headers

$(PRODUCT): $(OBJS)
	$(CC) -dynamiclib $(OBJS) $(LDFLAGS) -o $(PRODUCT)

$(FPREFIX)/$(notdir $(TARGET)): $(TARGET)
	$(RM) $(FPREFIX)/$(notdir $(TARGET))
	$(CP) $(TARGET) $(FPREFIX)

-include .depend
