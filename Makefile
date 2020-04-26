# native
CC?=gcc
PYTHON3?=python3

ifeq ($(CC),clang) # clang's lto has limitations, but we only have one .c anyways
CFLAGS?=-Wall -Werror -DWITH_ASSERT -D_FORTIFY_SOURCE=2 -pie -fPIE -ffunction-sections -fdata-sections -Wl,--gc-sections -s -Os
else
CFLAGS?=-Wall -Werror -DWITH_ASSERT -D_FORTIFY_SOURCE=2 -pie -fPIE -ffunction-sections -fdata-sections -Wl,--gc-sections -s -flto=4 -Os
endif

ifeq ($(OS),Windows_NT)
# native
WIN32CC?=$(CC)
WIN32CFLAGS?=$(CFLAGS)
WIN32WINDRES?=windres
else
# cross compile
WIN32CC?=i686-w64-mingw32-gcc
WIN32CFLAGS?=-Wall -DWITH_ASSERT -D_FORTIFY_SOURCE=2 -pie -fPIE -static -static-libgcc --function-sections -Wl,--gc-sections -s -fdata-sections -flto=4 -Os
WIN32WINDRES?=i686-w64-mingw32-windres
endif

PATCH_FILES := $(wildcard patches/*.txt) # this is replaced by pre-built gen.h in release to reduce build dependencies
SOURCE_FILES = main.c
INCLUDE_FILES = data.h patches.h gen.h tinymt64.h


.PHONY: clean clean-temps all native win32 test release

ifeq ($(OS),Windows_NT)
native: win32
win32: evermizer.exe ow-patch.exe
all: native
else
native: evermizer ow-patch
win32: evermizer.exe ow-patch.exe
all: native win32
endif

ifneq ($(PATCH_FILES),) # assume we have a pre-built gen.h if pathes/ is missing
gen.h: $(PATCH_FILES) everscript2h.py
	$(PYTHON3) everscript2h.py $@ $(PATCH_FILES)
endif

main.res: icon.ico
	echo "id ICON $(^)" | $(WIN32WINDRES) -O coff -o $@

evermizer: $(SOURCE_FILES) $(INCLUDE_FILES)
	$(CC) -o $@ $(SOURCE_FILES) $(CFLAGS)

ow-patch: $(SOURCE_FILES) $(INCLUDE_FILES)
	$(CC) -o $@ $(SOURCE_FILES) $(CFLAGS)

evermizer.exe: $(SOURCE_FILES) $(INCLUDE_FILES) main.res
	$(WIN32CC) -o $@ $(SOURCE_FILES) $(WIN32CFLAGS) main.res

ow-patch.exe: $(SOURCE_FILES) $(INCLUDE_FILES) main.res
	$(WIN32CC) -o $@ $(SOURCE_FILES) $(WIN32CFLAGS) main.res

clean: clean-temps
	rm -rf evermizer evermizer.exe ow-patch ow-patch.exe

clean-temps:
	rm -rf main.res
ifneq ($(PATCH_FILES),) # only if not pre-built
	rm -rf gen.h
endif

test: all
	./test.sh # this tests code, not your binary. only required for release

release: test
	./release.sh # this creates the zip, not required to run it

