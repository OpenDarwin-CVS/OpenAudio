# -*- Makefile -*-

FPREFIX?=/Library/Frameworks
KPREFIX?=/System/Library/Extensions

PRODUCT=OpenAudio
KEXT=$(SRCROOT)/$(PRODUCT).kext
FRAMEWORK=$(SRCROOT)/$(PRODUCT).framework

STR2PLIST=util/str2plist
MV=mv
RM=rm -rf
CP=cp -r
CC=gcc
CXX=g++
SYSCTL=/usr/sbin/sysctl
INSTALL=/usr/bin/install

OSMAJOR=$(shell $(SYSCTL) -n kern.osrelease | cut -d. -f1)
OSMINOR=$(shell $(SYSCTL) -n kern.osrelease | cut -d. -f2)

ifeq ($(RELEASE), yes)
ARCHFLAGS=-arch ppc -arch i386
CPPFLAGS+=-DNDEBUG
endif

ifeq ($(KERNEL), yes)
CPPFLAGS=-I/System/Library/Frameworks/Kernel.framework/Headers \
	-I/System/Library/Frameworks/Kernel.framework/Headers/bsd \
	-I$(SRCROOT)/include \
	-DKERNEL -DKERNEL_PRIVATE -Wall -W -Werror -Wno-unused-parameter \
	-DDARWIN_MAJOR=$(OSMAJOR) -DDARWIN_MINOR=$(OSMINOR)

KFLAGS=-no-cpp-precomp -static -fno-common -finline -fno-keep-inline-functions \
	-force_cpusubtype_ALL -Os $(ARCHFLAGS) -nostdinc -g -fno-common \
	-fmessage-length=0 -force_cpusubtype_ALL

CXXFLAGS=$(KFLAGS) -fapple-kext -fno-rtti -fno-exceptions -fcheck-new \
	 -msoft-float -fpermissive -mlong-branch

CFLAGS=$(KFLAGS) -fno-builtin

LDFLAGS=$(ARCHFLAGS) -static -nostdlib -r -lcc_kext -g -lkmodc++ -lkmod

else

endif
