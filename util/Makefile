
all: bswap str2plist

str2plist: str2plist.c
	$(CC) -framework CoreFoundation $? -o $@

bswap: bswap.c
	$(CC) $? -o $@
