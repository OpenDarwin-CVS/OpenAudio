#*- makefile -*-

PRODUCT=ODAudioSystem

TARGET=$(DSTROOT)/$(PRODUCT).kext
OBJS=module.o ODBSDClient.o ODAudioBSDClient.o
SRCS=$(shell for file in $(OBJS); do for ext in .cc .cpp .mm .c .m; do \
		test -e $${file%.o}$$ext && echo $${file%%.o}$$ext; \
	done; done)

include ../config.mak

.PHONY: .depend clean dist all load unload reload

.c.o:
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

.cc.o:
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

all: $(OBJS)

dist: all
	$(RM) $(TARGET)
	$(MKDIR) $(TARGET)/Contents/MacOS
	$(MKDIR) $(TARGET)/Contents/Resources
	chown -R root:wheel $(TARGET)
	chmod -R 755 $(TARGET)
	$(CC) $(OBJS) $(LDFLAGS) -o $(TARGET)/Contents/MacOS/$(PRODUCT)
	$(CP) Info.plist $(TARGET)/Contents

load: dist
	kextload -t -s $(TARGET)/Contents/Resources $(TARGET)

unload: dist
	kextunload $(TARGET)

reload: unload load

release:
	sudo $(MAKE) clean dist \
	ARCHFLAGS="-arch ppc -arch i386" CPPFLAGS="$(CPPFLAGS) -DNDEBUG"

clean:
	$(RM) $(OBJS) $(PRODUCT) $(PRODUCT).plist $(TARGET) .depend

.depend:
	gcc -MM $(CPPFLAGS) $(SRCS) > .depend

-include .depend