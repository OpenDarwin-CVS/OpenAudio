#*- makefile -*-

include config.mak

.SUFFIXES: .plist .strings

.PHONY: .depend clean dist all load unload

.c.o:
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

.cc.o:
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

.strings.plist:
	$(STR2PLIST) $< $@

all: $(PRODUCT)

dist: $(PRODUCT).kext/Contents/MacOS/$(PRODUCT)

load: dist
	kextload -t $(PRODUCT).kext

unload: dist
	kextunload $(PRODUCT).kext

clean:
	$(RM) $(OBJS) $(PRODUCT) $(PRODUCT).plist $(PRODUCT).kext
	$(RM) $(STR2PLIST) .depend

$(STR2PLIST): $(STR2PLIST).c
	$(CC) -framework CoreFoundation $? -o $@

$(PRODUCT): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(PRODUCT).kext/Contents/MacOS/$(PRODUCT): \
		$(STR2PLIST) $(PRODUCT) $(PRODUCT).plist
	$(MKDIR) $(PRODUCT).kext/Contents/MacOS
	$(MV) $(PRODUCT) $(PRODUCT).kext/Contents/MacOS/$(PRODUCT)
	$(MV) $(PRODUCT).plist $(PRODUCT).kext/Contents/Info.plist
	chown -R root:wheel $(PRODUCT).kext
	chmod -R 644 $(PRODUCT).kext

.depend:
	gcc -MM $(CPPFLAGS) $(SRCS) > .depend

-include .depend