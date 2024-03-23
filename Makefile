-include Makeconfig

# native compiler
CC?=gcc
# any python (2 or 3)
PYTHON?=python

ifeq ($(CC),clang) # clang's lto has limitations, but we only have one .c anyways
CFLAGS?=-Wall -Werror -DWITH_ASSERT -D_FORTIFY_SOURCE=2 -pie -fPIE -ffunction-sections -fdata-sections -Wl,--gc-sections -s -Os
else
CFLAGS?=-Wall -Werror -DWITH_ASSERT -D_FORTIFY_SOURCE=2 -pie -fPIE -ffunction-sections -fdata-sections -Wl,--gc-sections -s -flto=4 -Os
endif

ifeq ($(OS),Windows_NT)
# native, whatever architecture you are on
WIN32CC?=$(CC)
WIN32CFLAGS?=$(CFLAGS) -U_FORTIFY_SOURCE
WIN32WINDRES?=windres
# tools
MV?=move
CPPCHECK:=$(shell where cppcheck)
CLOSURECOMPILER:=$(shell where google-closure-compiler)
SED:=$(shell where sed)
else
# cross compile, we build both 32bit and 64bit
WIN32CC?=i686-w64-mingw32-gcc
WIN32CFLAGS?=-Wall -Werror -DWITH_ASSERT -pie -fPIE -static -static-libgcc -ffunction-sections -fdata-sections -Wl,--gc-sections -s -flto=4 -Os
WIN32WINDRES?=i686-w64-mingw32-windres
WIN64CC?=x86_64-w64-mingw32-gcc
WIN64CFLAGS?=$(WIN32CFLAGS)
WIN64WINDRES?=x86_64-w64-mingw32-windres
# tools
MV?=mv
CPPCHECK:=$(shell which cppcheck)
CLOSURECOMPILER:=$(shell which google-closure-compiler)
SED:=$(shell which sed)
endif

CPPCHECK_OPT=--enable=all --check-level=exhaustive --suppress=missingIncludeSystem --suppress=duplicateExpression:patches.h --inline-suppr -q
CPPCHECKS=cppcheck1 cppcheck2 cppcheck3 cppcheck4 cppcheck5 cppcheck6 cppcheck7 cppcheck8 cppcheck9 cppcheck10 cppcheck11 cppcheck12 cppcheck13 cppcheck14 cppcheck15 cppcheck16

EMCC?=emcc
EMFLAGS?=-D NO_ASSERT -s ENVIRONMENT=web -s WASM=1 -s INVOKE_RUN=0 -s EXIT_RUNTIME=0 -s ASSERTIONS=0 -s TOTAL_STACK=1048576 -s INITIAL_MEMORY=6291456 -Os --closure 0
# NOTE: em's --closure 1 may fail/remove too much or clash with other code, we call closure compiler manually for that reason

PATCH_FILES := $(wildcard patches/*.txt) # this is replaced by pre-built gen.h in release to reduce build dependencies
IPS_INFO_FILES := $(wildcard ips/*.txt) # this is replaced by pre-built gen.h in release to reduce build dependencies
SOURCE_FILES = main.c
INCLUDE_FILES = rng.h util.h data.h sniff.h gourds.h doggo.h patches.h gen.h tinymt64.h


.PHONY: clean clean-temps all native win32 wasm test test-code $(CPPCHECKS) test-code-run release

ifeq ($(OS),Windows_NT)
native: win32
win32: evermizer.exe
wasm: evermizer.js
all: native wasm
EXE = evermizer.exe
else
native: evermizer
win32: evermizer.exe
win64: evermizer64.exe
wasm: evermizer.js
all: native win32 win64 wasm
EXE = evermizer
endif

ifneq ($(strip $(PATCH_FILES) $(IPS_INFO_FILES)),) # assume we have a pre-built gen.h if patches/ and ips/ is missing
gen.h: $(PATCH_FILES) everscript2h.py $(IPS_INFO_FILES) ips2h.py
	$(PYTHON) everscript2h.py $@ $(PATCH_FILES)
	$(PYTHON) ips2h.py -a $@ $(IPS_INFO_FILES)
endif
ifneq (,$(wildcard gourds.csv)) # assume we have a pre-built gourds.h if gourds.csv is missing
gourds.h: gourds.csv gourds2h.py util.py
ifeq ($(strip $(ROM)),)
	$(PYTHON) gourds2h.py $@ gourds.csv
else
	$(PYTHON) gourds2h.py $@ gourds.csv "$(ROM)"
endif
endif
ifneq (,$(wildcard gourds.csv)) # assume we have a pre-built sniff.h if sniff.csv is missing
sniff.h: sniff.csv sniff2h.py util.py
ifeq ($(strip $(ROM)),)
	$(PYTHON) sniff2h.py $@ sniff.csv
else
	$(PYTHON) sniff2h.py $@ sniff.csv "$(ROM)"
endif
endif

main.res: icon.ico
	echo "id ICON $(^)" | $(WIN32WINDRES) -O coff -o $@

main64.res: icon.ico
	echo "id ICON $(^)" | $(WIN64WINDRES) -O coff -o $@

evermizer: $(SOURCE_FILES) $(INCLUDE_FILES)
	$(CC) -o $@ $(SOURCE_FILES) $(CFLAGS)

evermizer.exe: $(SOURCE_FILES) $(INCLUDE_FILES) main.res
	$(WIN32CC) -o $@ $(SOURCE_FILES) $(WIN32CFLAGS) main.res

evermizer64.exe: $(SOURCE_FILES) $(INCLUDE_FILES) main64.res
	$(WIN64CC) -o $@ $(SOURCE_FILES) $(WIN64CFLAGS) main64.res

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
	rm -rf evermizer evermizer.exe evermizer64.exe ow-patch ow-patch.exe ow-patch64.exe evermizer.html evermizer.js evermizer.wasm

clean-temps:
	rm -rf main.res main64.res
ifneq ($(strip $(PATCH_FILES) $(IPS_INFO_FILES)),) # only if not pre-built
	rm -rf gen.h
endif
ifneq (,$(wildcard gourds.csv)) # only if not pre-built
	rm -rf gourds.h
endif
ifneq (,$(wildcard sniff.csv)) # only if not pre-built
	rm -rf sniff.h
endif

test: $(EXE)
ifeq ($(strip $(ROM)),)
	$(error ROM is not set. Set it in Makeconfig, environment or make variable)
endif
	$(PYTHON) test.py "./$(EXE)" "$(ROM)"

ifneq ($(strip $(CPPCHECK)),)
# latest cppcheck takes ~5min. make release with -j8 or higher
cppcheck1:
	$(CPPCHECK) $(CPPCHECK_OPT) -D__linux__ main.c
cppcheck2:
	$(CPPCHECK) $(CPPCHECK_OPT) -D__linux__ -DWITH_ASSERT main.c
cppcheck3:
	$(CPPCHECK) $(CPPCHECK_OPT) -D__linux__ -DNO_UI main.c
cppcheck4:
cppcheck5:
	$(CPPCHECK) $(CPPCHECK_OPT) -D_WIN32 main.c
cppcheck6:
	$(CPPCHECK) $(CPPCHECK_OPT) -D_WIN32 -DWITH_ASSERT main.c
cppcheck7:
	$(CPPCHECK) $(CPPCHECK_OPT) -D_WIN32 -DNO_UI main.c
cppcheck8:
cppcheck9:
	$(CPPCHECK) $(CPPCHECK_OPT) -D_WIN32 -D_MSC_VER main.c
cppcheck10:
	$(CPPCHECK) $(CPPCHECK_OPT) -D_WIN32 -D_MSC_VER -DWITH_ASSERT main.c
cppcheck11:
	$(CPPCHECK) $(CPPCHECK_OPT) -D_WIN32 -D_MSC_VER -DNO_UI main.c
cppcheck12:
cppcheck13:
	$(CPPCHECK) $(CPPCHECK_OPT) -D__EMSCRIPTEN__ main.c
cppcheck14:
	$(CPPCHECK) $(CPPCHECK_OPT) -D__EMSCRIPTEN__ -DWITH_ASSERT main.c
cppcheck15:
	$(CPPCHECK) $(CPPCHECK_OPT) -D__EMSCRIPTEN__ -DNO_UI main.c
cppcheck16:
else
cppcheck1: ;
cppcheck2: ;
cppcheck3: ;
cppcheck4: ;
cppcheck5: ;
cppcheck6: ;
cppcheck7: ;
cppcheck8: ;
cppcheck9: ;
cppcheck10: ;
cppcheck11: ;
cppcheck12: ;
cppcheck13: ;
cppcheck14: ;
cppcheck15: ;
cppcheck16: ;
endif

test-code-run:
ifeq ($(strip $(ROM)),)
	$(error ROM is not set. Set it in Makeconfig, environment or make variable)
endif
	./test-code.sh "$(ROM)" # this tests code, not your binary. only required for release

test-code: $(CPPCHECKS) test-code-run ;

release: all test test-code
	./release.sh # this creates the zip, not required to run it

