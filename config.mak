# -*- Makefile -*-

AUDIO_ENGINE_NAME:=$(shell ioreg | egrep 'Audio.*Engine' | \
			   tr -d \| | tr -s \  | cut -d \  -f 3 | tail -n 1)

PRODUCT=DarwinSound
OBJS=ds_module.o ds_init.o

MKDIR=mkdir -p
STR2PLIST=util/str2plist
CP=cp
MV=mv
RM=rm -rf
CC=gcc
CXX=g++
SYSCTL=/usr/sbin/sysctl

OSMAJOR=$(shell $(SYSCTL) -n kern.osrelease | cut -d. -f1)
OSMINOR=$(shell $(SYSCTL) -n kern.osrelease | cut -d. -f2)

ARCHFLAGS=-arch ppc #-arch i386

CPPFLAGS=-I/System/Library/Frameworks/Kernel.framework/Headers \
	-I/System/Library/Frameworks/Kernel.framework/Headers/bsd \
	-DKERNEL -DKERNEL_PRIVATE -Wall -W -Wno-unused-parameter \
	-DDARWIN_MAJOR=$(OSMAJOR) -DDARWIN_MINOR=$(OSMINOR) \
	-DAUDIO_ENGINE_NAME=\"$(AUDIO_ENGINE_NAME)\"

KFLAGS=-no-cpp-precomp -static -fno-common -finline -fno-keep-inline-functions \
	-force_cpusubtype_ALL -Os $(ARCHFLAGS) -nostdinc -g -fno-common \
	-fmessage-length=0 -force_cpusubtype_ALL

CXXFLAGS=$(KFLAGS) -fapple-kext -fno-rtti -fno-exceptions -fcheck-new \
	 -msoft-float -fpermissive -mlong-branch

CFLAGS=$(KFLAGS) -fno-builtin

LDFLAGS=$(ARCHFLAGS) -static -nostdlib -r -lcc_kext -lkmodc++ -lkmod

