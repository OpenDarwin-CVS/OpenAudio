#*- makefile -*-
#
# Copyright (c) 2004, Dan Villiom Podlaski Christiansen
# See the file LICENSE for details
#

all: bswap str2plist

str2plist: str2plist.c
	$(CC) -framework CoreFoundation $? -o $@

bswap: bswap.c
	$(CC) $? -o $@
