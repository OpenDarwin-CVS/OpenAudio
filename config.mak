# -*- makefile -*-
#
# Copyright (c) 2004 Dan Villiom Podlaski Christiansen <danchr@opendarwin.org>
# See the file LICENSE for details
#

FPREFIX?=/Library/Frameworks
KPREFIX?=/System/Library/Extensions
EPREFIX?=/usr/local/bin

PRODUCT=OpenAudio
KEXT=$(SRCROOT)/$(PRODUCT).kext
FRAMEWORK=$(SRCROOT)/$(PRODUCT).framework

STR2PLIST=util/str2plist
MV=mv
RM=rm -rf
CP=cp -r
CC=gcc
CXX=g++
INSTALL=/usr/bin/install

ifeq ($(KERNEL), yes)
CPPFLAGS=-I/System/Library/Frameworks/Kernel.framework/Headers \
	-I/System/Library/Frameworks/Kernel.framework/Headers/bsd \
	-DKERNEL -DKERNEL_PRIVATE -Wall -W -Werror -Wno-unused-parameter

KFLAGS=-static -fno-common -finline -fno-keep-inline-functions \
	-force_cpusubtype_ALL -Os $(ARCHFLAGS) -nostdinc -g -fno-common \
	-fmessage-length=0 -force_cpusubtype_ALL

CXXFLAGS=$(KFLAGS) -fapple-kext -fno-rtti -fno-exceptions -fcheck-new \
	 -msoft-float -fpermissive -mlong-branch

CFLAGS=$(KFLAGS) -fno-builtin

LDFLAGS=$(ARCHFLAGS) -static -nostdlib -r -lcc_kext -g -lkmodc++ -lkmod

else

CPPFLAGS=-W -Wall -Werror -Wno-unused-parameter
LDFLAGS=-framework CoreFoundation $(ARCHFLAGS)
CFLAGS=-std=c99 -pedantic -O3 $(ARCHFLAGS)
CXXFLAGS=-O3 $(ARCHFLAGS)

endif

ifeq ($(RELEASE), yes)
ARCHFLAGS=$(foreach i, ppc i386, \
	$(shell test -d /usr/libexec/gcc/darwin/$i && echo "-arch $i"))
CPPFLAGS+=-DNDEBUG
endif
