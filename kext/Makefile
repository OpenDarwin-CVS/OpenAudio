#*- makefile -*-

include config.mak

.PHONY: .depend clean dist all load unload reload

.c.o:
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

.cc.o:
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

all: $(OBJS)

dist: all
	$(RM) $(PRODUCT).kext
	$(MKDIR) $(PRODUCT).kext/Contents/MacOS
	$(MKDIR) $(PRODUCT).kext/Contents/Resources
	chown -R root:wheel $(PRODUCT).kext
	chmod -R 755 $(PRODUCT).kext
	$(CC) $(OBJS) $(LDFLAGS) -o $(PRODUCT).kext/Contents/MacOS/$(PRODUCT)
	$(CP) Info.plist $(PRODUCT).kext/Contents

load: dist
	kextload -t -s $(PRODUCT).kext/Contents/Resources $(PRODUCT).kext

unload: dist
	kextunload $(PRODUCT).kext

reload: unload load

release:
	sudo $(MAKE) clean dist \
	ARCHFLAGS="-arch ppc -arch i386" CPPFLAGS="$(CPPFLAGS) -DNDEBUG"

clean:
	$(RM) $(OBJS) $(PRODUCT) $(PRODUCT).plist $(PRODUCT).kext .depend

.depend:
	gcc -MM $(CPPFLAGS) $(SRCS) > .depend

-include .depend