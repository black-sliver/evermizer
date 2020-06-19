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
# tools
MV?=move
CPPCHECK:=$(shell where cppcheck)
CLOSURECOMPILER:=$(shell where closure-compiler)
SED:=$(shell where sed)
else
# cross compile
WIN32CC?=i686-w64-mingw32-gcc
WIN32CFLAGS?=-Wall -Werror -DWITH_ASSERT -D_FORTIFY_SOURCE=2 -pie -fPIE -static -static-libgcc -ffunction-sections -fdata-sections -Wl,--gc-sections -s -flto=4 -Os
WIN32WINDRES?=i686-w64-mingw32-windres
# tools
MV?=mv
CPPCHECK:=$(shell which cppcheck)
CLOSURECOMPILER:=$(shell which closure-compiler)
SED:=$(shell which sed)
endif

EMCC?=emcc
EMFLAGS?=-D NO_ASSERT -s WASM=1 -s INVOKE_RUN=0 -s EXIT_RUNTIME=0 -s ASSERTIONS=0 -Os #--closure 1

PATCH_FILES := $(wildcard patches/*.txt) # this is replaced by pre-built gen.h in release to reduce build dependencies
SOURCE_FILES = main.c
INCLUDE_FILES = rng.h util.h data.h sniff.h gourds.h patches.h gen.h tinymt64.h


.PHONY: clean clean-temps all native win32 wasm test release

ifeq ($(OS),Windows_NT)
native: win32
win32: evermizer.exe ow-patch.exe
wasm: evermizer.js
all: native wasm
else
native: evermizer ow-patch
win32: evermizer.exe ow-patch.exe
wasm: evermizer.js
all: native win32 wasm
endif

ifneq ($(strip $(PATCH_FILES)),) # assume we have a pre-built gen.h if patches/ is missing
gen.h: $(PATCH_FILES) everscript2h.py
	$(PYTHON3) everscript2h.py $@ $(PATCH_FILES)
endif
ifneq (,$(wildcard gourds.csv)) # assume we have a pre-built gourds.h if gourds.csv is missing
gourds.h: gourds.csv gourds2h.py
	$(PYTHON3) gourds2h.py $@ gourds.csv $(ROM)
endif

main.res: icon.ico
	echo "id ICON $(^)" | $(WIN32WINDRES) -O coff -o $@

evermizer: $(SOURCE_FILES) $(INCLUDE_FILES)
	$(CC) -o $@ $(SOURCE_FILES) $(CFLAGS)

ow-patch: $(SOURCE_FILES) $(INCLUDE_FILES)
	$(CC) -DNO_RANDO -o $@ $(SOURCE_FILES) $(CFLAGS)

evermizer.exe: $(SOURCE_FILES) $(INCLUDE_FILES) main.res
	$(WIN32CC) -o $@ $(SOURCE_FILES) $(WIN32CFLAGS) main.res

ow-patch.exe: $(SOURCE_FILES) $(INCLUDE_FILES) main.res
	$(WIN32CC) -DNO_RANDO -o $@ $(SOURCE_FILES) $(WIN32CFLAGS) main.res

evermizer.js: $(SOURCE_FILES) $(INCLUDE_FILES)
	$(EMCC) -DNO_UI -o $@ $(SOURCE_FILES) $(EMFLAGS)
ifneq ($(strip $(SED)),) # reduce emscripten verbosity, skip if no sed installed
	$(SED) -i "s/Arguments to path.resolve must be strings/args must be strings/g" $@
	$(SED) -i "s/Unknown file open mode/Unknown mode/g" $@
	$(SED) -i "s/operations in flight at once, probably just doing extra work/ops active/g" $@
	$(SED) -i "s/threw an exception/threw/g" $@
	$(SED) -i "s/encoding type/encoding/g" $@
	$(SED) -i "s/Lazy loading should have been performed (contents set) in createLazyFile, but it was not. Lazy loading only works in web workers. Use --embed-file or --preload-file in emcc on the main thread/No lazy loading/g" $@
	$(SED) -i "s/Use --embed-file or --preload-file in emcc//g" $@
	$(SED) -i 's/out("LazyFiles on gzip forces download of the whole file when length is accessed");//g' $@
endif
ifneq ($(strip $(CLOSURECOMPILER)),) # skip if not installed, also see EMFLAGS
	$(CLOSURECOMPILER) --compilation_level SIMPLE_OPTIMIZATIONS --js="$@" --js_output_file="evermizer.min.js"
	$(MV) "evermizer.min.js" "$@"
endif

clean: clean-temps
	rm -rf evermizer evermizer.exe ow-patch ow-patch.exe evermizer.html evermizer.js evermizer.wasm

clean-temps:
	rm -rf main.res
ifneq ($(strip $(PATCH_FILES)),) # only if not pre-built
	rm -rf gen.h
endif
ifneq (,$(wildcard gourds.csv)) # only if not pre-built
	rm -rf gourds.h
endif

test: all
ifneq ($(strip $(CPPCHECK)),)
	$(CPPCHECK) --force --enable=all --suppress=missingIncludeSystem --suppress=duplicateExpression -q main.c
endif
	./test.sh # this tests code, not your binary. only required for release

release: test
	./release.sh # this creates the zip, not required to run it

