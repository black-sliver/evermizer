// yes, this is a mess, but I really want to make single-c-file work
// this is the simplest way to get a static EXE and WASM
// from a common source, also without requiring project/make files
// see google doc for documentation on the actual patches used below
// the idea is to "manually" patch the game to a state where we simply swap
// out some numbers to make it random (without rewriting/relocating everything)
#define VERSION "v026d"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <strings.h> // strcasecmp
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h> 
#include "tinymt64.h"
#ifndef NO_UI // includes and helpers for UI
#if defined(WIN32) || defined(_WIN32)
#include <process.h>
#include <conio.h>
#define clrscr() system("cls");
#else
#include <termios.h>
#include <unistd.h>
#define clrscr() printf("\e[1;1H\e[2J")
char getch() {
    static struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    char c = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
    return c;
}
#endif
#endif

#if defined(WIN32) || defined(_WIN32)
#define DIRSEP '\\'
#ifndef PRIx64
#define PRIx64 "I64x"
#define uint64_t unsigned __int64
#endif
#else
#define DIRSEP '/'
#endif

#include "util.h"

#ifndef NO_RANDO
static bool stris(const char* s, const char* t) { if (s==t) return true; return strcmp(s,t)==0; }
#endif
#ifdef NO_UI
static bool batch = true;
#else
static bool batch = false;
#endif
void die(const char* msg)
{
    if (msg) fprintf(stderr, "%s", msg);
#if (defined(WIN32) || defined(_WIN32)) && !defined(NO_UI)
    if (!batch) system("pause");
#else
    (void)batch; // ignore warnings
#endif
    exit(1);
}
const char B32[] = "abcdefghijklmnopqrstuvwxyz234567=";
char b32(unsigned v) { return B32[v&0x1f]; }
#define APPLY_PATCH(buf, patch, loc) memcpy(buf+loc, patch, sizeof(patch)-1)

#include "rng.h"

#ifndef NO_RANDO
#include "gourds.h" // generated list of gourds and gourd drops
#endif
#include "data.h"


#define float DO_NOT_USE_FLOATS
#define double DO_NOT_USE_FLOATS


// Misc consts
const char DIFFICULTY_CHAR[] = {'e','n','h'};
const char* const DIFFICULTY_NAME[] = {"Easy","Normal","Hard"};
#define DEFAULT_difficulty 1
struct option { char key; bool def; const char* text; const char* info; };
const static struct option options[] = {
#ifndef NO_RANDO
    { 'c', false, "Chaos", NULL },
#endif
    { 0,   true,  "Open World", NULL },
    { 'k', true,  "Keep dog", NULL },
    { '1', true,  "Fix sequence", NULL },
    { '2', true,  "Fix cheats", NULL },
#ifndef NO_RANDO
    { '3', true,  "Glitchless beatable", NULL },
    { 'a', true,  "Alchemizer", NULL },
    { 'i', true,  "Ingredienizer", NULL },
    { 'b', true,  "Boss dropamizer", NULL },
    { 'g', true,  "Gourdomizer", NULL },
    { 's', true,  "Sniffamizer", NULL },
    { 'd', false, "Doggomizer", "Act1-3" },
    { 'm', false, "Musicmizer", "Demo" },
    { 'l', false, "Spoiler Log", NULL },
#endif
};
enum option_indices {
#ifndef NO_RANDO
    chaos_idx,
#endif
    openworld_idx, keepdog_idx, fixsequence_idx, fixcheats_idx,
#ifndef NO_RANDO
    glitchless_idx, alchemizer_idx, ingredienizer_idx, bossdropamizer_idx,
    gourdomizer_idx, sniffamizer_idx, doggomizer_idx, /*enemizer_idx,*/
    musicmizer_idx, spoilerlog_idx
#endif
};
#define D(IDX) options[ IDX ].def
#define O(IDX) option_values[ IDX ]
#define C(IDX) ( O(IDX) != D(IDX) )
#define chaos O(chaos_idx)
#define openworld O(openworld_idx)
#define keepdog O(keepdog_idx)
#define fixsequence O(fixsequence_idx)
#define fixcheats O(fixcheats_idx)
#define glitchless O(glitchless_idx)
#define alchemizer O(alchemizer_idx)
#define ingredienizer O(ingredienizer_idx)
#define bossdropamizer O(bossdropamizer_idx)
#define gourdomizer O(gourdomizer_idx)
#define sniffamizer O(sniffamizer_idx)
#define doggomizer O(doggomizer_idx)
#define enemizer O(enemizer_idx)
#define musicmizer O(musicmizer_idx)
#define spoilerlog O(spoilerlog_idx)

#define DEFAULT_OW() do {\
    for (size_t i=0; i<ARRAY_SIZE(options); i++)\
        O(i) = D(i);\
} while (false)
#define DEFAULT_RANDO() do {\
    difficulty=DEFAULT_difficulty;\
    DEFAULT_OW();\
} while (false)
#ifdef NO_RANDO
#define DEFAULT_SETTINGS() DEFAULT_OW()
#else
#define DEFAULT_SETTINGS() DEFAULT_RANDO()
#endif

#ifndef NO_RANDO
#define SETTINGS2STR(s)\
    do {\
        char* t = s;\
        assert(ARRAY_SIZE(s)>ARRAY_SIZE(options)+2);\
        *t++ = 'r'; *t++ = DIFFICULTY_CHAR[difficulty];\
        for (size_t i=0; i<ARRAY_SIZE(options); i++)\
            if (C(i) && options[i].key) *t++ = options[i].key;\
        *t++ = 0;\
    } while (false)
#else
#define SETTINGS2STR(s)\
    do {\
        char* t = s;\
        assert(ARRAY_SIZE(s)>ARRAY_SIZE(options)+2);\
        *t++ = 'r';\
        for (size_t i=0; i<ARRAY_SIZE(options); i++)\
            if (C(i) && options[i].key) *t++ = options[i].key;\
        *t++ = 0;\
    } while (false)
#endif
#ifdef NO_UI
#define FLAGS "[-o <dst file.sfc>|-d <dst directory>] "
#else
#define FLAGS "[-b|-i] [-o <dst file.sfc>|-d <dst directory>] "
#endif
#ifdef NO_RANDO
#define APPNAME "SoE-OpenWorld"
#define ARGS " [settings]"
#else
#define APPNAME "Evermizer"
#define ARGS " [settings [seed]]"
#endif

// The actual program
static void print_usage(const char* appname)
{
    fprintf(stderr, "Usage: %s " FLAGS "<src file.sfc>" ARGS "\n", appname);
    fprintf(stderr, "       %s --help             show this output\n", appname);
    fprintf(stderr, "       %s --version          print version\n", appname);
    fprintf(stderr, "       %s --verify <rom.sfc> check if rom is compatible\n", appname);
#ifndef __EMSCRIPTEN__
    fprintf(stderr, "       %s --settings         show available settings\n", appname);
    fprintf(stderr, "       %s --settings.json    above as json\n", appname);
#else
    fprintf(stderr, "       %s --settings.json    available settings as json\n", appname);
#endif
#if defined(WIN32) || defined(_WIN32)
    fprintf(stderr, "       or simply drag & drop your ROM onto the EXE\n");
#ifndef NO_UI
    if (!batch) system("pause");
#endif
#endif
}
#ifndef __EMSCRIPTEN__
static void print_settings()
{
    printf("%s %s settings:\n", APPNAME, VERSION);
#ifndef NO_RANDO
    printf("Difficulty:\n");
    for (uint8_t i=0; i<ARRAY_SIZE(DIFFICULTY_CHAR); i++)
        printf("  %c: %s%s\n", DIFFICULTY_CHAR[i], DIFFICULTY_NAME[i],
                               i==DEFAULT_difficulty?" (default)":"");
#endif
    printf("Options:\n");
    for (size_t i=0; i<ARRAY_SIZE(options); i++) {
        const struct option* opt = options+i;
        if (! opt->key) continue;
        printf("  %c: %s %s%s%s%s\n", opt->key, opt->def?"No":"  ",opt->text,
                  opt->info?" [":"", opt->info?opt->info:"", opt->info?"]":"");
    }
    printf("\n");
}
#endif
static void print_settings_json()
{
#ifndef NO_RANDO
    printf("{\"Difficulty\": [\n");
    for (uint8_t i=0; i<ARRAY_SIZE(DIFFICULTY_CHAR); i++) {
        if (i != 0) printf(",\n");
        printf("  [ \"%c\", \"%s\", %s ]", DIFFICULTY_CHAR[i], DIFFICULTY_NAME[i],
                               i==DEFAULT_difficulty?"true":"false");
    }
    printf("\n ],\n");
#endif
    printf(" \"Options\": [\n");
    for (size_t i=0; i<ARRAY_SIZE(options); i++) {
        const struct option* opt = options+i;
        if (! opt->key) continue;
        if (i != 0) printf(",\n");
        printf("  [ \"%c\", \"%s%s%s%s\", %s ]", opt->key, opt->text,
                  opt->info?" [":"", opt->info?opt->info:"", opt->info?"]":"",
                  opt->def?"true":"false");
    }
    printf("\n ]\n}\n\n");
}
int main(int argc, const char** argv)
{
#ifndef __EMSCRIPTEN__
    const char* appname = argv[0];
#else
    const char* appname = APPNAME;
#endif
    
    // verify at least one agument is given
    if (argc<2 || !argv[1] || !argv[1][0]) {
        print_usage(appname);
        die(NULL);
    }
    
    #ifdef NO_RANDO // open world + fixes only
    (void)alchemy_locations; // suppress warning
    #else // only in rando
    uint8_t difficulty;
    #endif
    bool option_values[ARRAY_SIZE(options)];
    
    // default settings
    #if !defined NO_RANDO && !defined NO_UI // only rando has interactive mode (yet)
    bool interactive = true; // show settings ui
    #endif
    bool verify = false; // verify ROM and exit
    DEFAULT_SETTINGS();
    
    const char* ofn=NULL;
    const char* dstdir=NULL;
    bool modeforced=false;
    
    // parse command line arguments
    while (argc>1) {
        if (strcmp(argv[1], "-b") == 0) {
            modeforced = true;
            batch = true;
    #if !defined NO_UI && !defined NO_RANDO // NO_RANDO has no UI (yet)
            interactive = false;
    #endif
            argv++; argc--;
        } else if (strcmp(argv[1], "-i") == 0) {
            modeforced = true;
    #if defined NO_UI || defined NO_RANDO  // NO_RANDO has no UI (yet)
            fprintf(stderr, "Requested interactive mode, but not compiled in!\n");
            print_usage(appname);
            die(NULL);
    #endif
            argv++; argc--;
        } else if (strcmp(argv[1], "-o") == 0) {
            ofn = argv[2];
            argv+=2; argc-=2;
            if (dstdir) die("Can't have -o and -d\n");
        } else if (strcmp(argv[1], "-d") == 0) {
            dstdir = argv[2];
            argv+=2; argc-=2;
            if (ofn) die("Can't have -o and -d\n");
        } else if (strcmp(argv[1], "--version") == 0) {
            printf("%s\n", VERSION);
            return 0;
        } else if (strcmp(argv[1], "--help") == 0) {
            print_usage(appname);
            return 0;
    #ifndef __EMSCRIPTEN__
        } else if (strcmp(argv[1], "--settings") == 0) {
            print_settings();
            return 0;
    #endif
        } else if (strcmp(argv[1], "--settings.json") == 0) {
            print_settings_json();
            return 0;
        } else if (strcmp(argv[1], "--verify") == 0) {
            argv++; argc--;
            verify=true;
        } else {
            break;
        }
    }
    
    if (!modeforced) {
        #if defined NO_RANDO && !defined NO_UI
        //interactive = argc<3; // NO_RANDO has no UI (yet)
        #elif !defined NO_UI
        interactive = argc<4;
        #endif
    }
    
    // verify number of command line arguments
    #ifdef NO_RANDO
    if (argc<2 || !argv[1] || !argv[1][0] || argc>3) {
    #else
    if (argc<2 || !argv[1] || !argv[1][0] || argc>4) {
    #endif
        print_usage(appname);
        die(NULL);
    }
    
    // parse settings command line argument
    if (argc>=3) {
        for (const char* s=argv[2]; *s; s++) {
            char c = tolower(*s);
        #ifndef NO_RANDO
            for (size_t i=0; i<ARRAY_SIZE(DIFFICULTY_CHAR); i++) {
                if (c == DIFFICULTY_CHAR[i]) { difficulty = i; c = 0; }
            }
        #endif
            for (size_t i=0; i<ARRAY_SIZE(options); i++) {
                if (options[i].key && c == options[i].key) { option_values[i] = !option_values[i]; c=0; break; }
            }
            if (c == 'r') DEFAULT_SETTINGS();
            else if (c != 0) {
                fprintf(stderr, "Unknown setting '%c' in \"%s\"\n",
                        c, argv[2]);
                die(NULL);
            }
        }
    }
    
    // parse source file command line argument
    const char* src = argv[1];
    
    // load rom
    FILE* fsrc = fopen(src,"rb");
    if (!fsrc) die("Could not open input file!\n");
    uint8_t zbuf[5] = {0,0,0,0,0};
    if (fread(zbuf, sizeof(*zbuf), sizeof(zbuf), fsrc) && zbuf[0]=='P' &&
            zbuf[1]=='K' && zbuf[2]==3 && zbuf[3]==4 && zbuf[4]>=10)
    {
        fclose(fsrc);
        die("Compressed ROMs not supported! Please extract first!\n");
    }
    fseek(fsrc, 0L, SEEK_END);
    size_t sz = ftell(fsrc);
    if (sz != 3145728 && sz != 3145728+512) {
        fclose(fsrc);
        die("ROM has to be 3MB SFC with or without header!\n");
    }
    fseek(fsrc, 0L, SEEK_SET);
    
    const size_t rom_off = (sz == 3145728+512) ? 512 : 0;
    bool grow = false; // will be set by patches if gowing the rom is required
    #define GROW_BY (1024*1024) // 1MB to a round 4MB
    
    uint8_t* buf = (uint8_t*)malloc(sz+GROW_BY); // allow to grow by 1MB
    memset(buf+sz, 0, GROW_BY); // or 0xff?
    size_t len = fread(buf, 1, sz, fsrc);
    if (len!=sz) { free(buf); fclose(fsrc); die("Could not read input file!\n"); }
    
    // check ROM header
    const char cart_header[] = "SECRET OF EVERMORE   \x31\x02\x0c\x03\x01\x33\x00";
    const size_t cart_header_loc = 0xFFC0;
    if (memcmp((char*)buf + rom_off + cart_header_loc, cart_header, sizeof(cart_header)-1) != 0)
    {
        size_t i = rom_off+cart_header_loc + 0x15;
        fprintf(stderr, "Wrong Header: %.21s %02x %02x %02x %02x %02x %02x %02x\n"
                        "Expected:     SECRET OF EVERMORE    31 02 0c 03 01 33 00\n",
                        (char*)buf+rom_off+cart_header_loc,
                        buf[i+0], buf[i+1], buf[i+2], buf[i+3],
                        buf[i+4], buf[i+5], buf[i+6]);
        free(buf);
        fclose(fsrc);
        die(NULL);
    }
    if (verify) {
        printf("OK");
        free(buf);
        fclose(fsrc);
        return 0;
    }
    
    // show command line settings in batch mode
    char settings[17];
    //if (argc>2) strncpy(settings, argv[2], sizeof(settings)); else memcpy(settings, "rn", 3);
    SETTINGS2STR(settings);
    
    #ifndef NO_RANDO
    // random seed number
    time_t t = 0;
    srand64((uint64_t)time(&t));
    uint64_t seed = rand64();
    
    // parse command line seed number
    if (argc>3 && argv[3] && isxdigit(argv[3][0]))
        seed = strtoull(argv[3], NULL, 16);
    
    // show UI in interactive mode
    #ifndef NO_UI // TODO: UI for OW
    if (interactive)
    {
        clrscr();
        printf(APPNAME " " VERSION "\n");
        if (argc<4) {
            char seedbuf[17];
            printf("Seed (ENTER for random): ");
            fflush(stdout);
            if (! fgets(seedbuf, sizeof(seedbuf), stdin)) die("\nAborting...\n");
            if (isxdigit(seedbuf[0])) seed = strtoull(seedbuf, NULL, 16);
        }
        while (true) {
            clrscr();
            printf(APPNAME " " VERSION "\n");
            printf("Seed: %" PRIx64 "\n", seed);
            SETTINGS2STR(settings);
            printf("Settings: %-18s(Press R to reset)\n", settings);
            printf("\n");
            printf("Difficulty:          %-6s (%c",
                    DIFFICULTY_NAME[difficulty], toupper(DIFFICULTY_CHAR[0]));
            for (size_t i=1; i<ARRAY_SIZE(DIFFICULTY_CHAR); i++)
                printf("/%c", toupper(DIFFICULTY_CHAR[i]));
            printf(" to change)\n");
            for (size_t i=0; i<ARRAY_SIZE(options); i++) {
                const struct option* opt = options+i;
                char col1[32]; snprintf(col1, sizeof(col1), "%s:", opt->text);
                printf("%-20s %s    %c%c%s%s%s%s\n", col1, O(i)?"on ":"off",
                        opt->key?'(':' ', opt->key?toupper(opt->key):' ', opt->key?" to toggle)":"",
                        opt->info?" [":"", opt->info?opt->info:"", opt->info?"]":"");
            }
            printf("\n");
            printf("Press ESC to abort, ENTER to continue");
            fflush(stdout);
            char c = getch();
            if (c == '\x1b' || c == EOF || c == '\x04' || tolower(c) == 'q') {
                fclose(fsrc); free(buf);
                die("\nAborting...\n");
            }
            if (c == '\r' || c == '\n') break;
            c = tolower(c);
            for (size_t i=0; i<ARRAY_SIZE(DIFFICULTY_CHAR); i++)
                if (c == DIFFICULTY_CHAR[i]) difficulty = i;
            for (size_t i=0; i<ARRAY_SIZE(options); i++)
                if (c == options[i].key) option_values[i] = !option_values[i];
            if (c == 'r') DEFAULT_SETTINGS();
        }
        clrscr();
    }
    #endif
    printf(APPNAME " " VERSION "\n");
    printf("Seed: %" PRIx64 "\n", seed);
    srand64(seed);
    bool randomized = alchemizer || ingredienizer || bossdropamizer ||
                      gourdomizer || sniffamizer || doggomizer /*||enemizer*/;
    #else
    printf("SoE OpenWorld " VERSION "\n");
    #endif
    printf("Settings: %-10s\n\n", settings);
    
    // define patches
    #define DEF_LOC(n, location)\
        const size_t PATCH_LOC##n = location
    #define DEF(n, location, content)\
        const size_t PATCH_LOC##n = location;\
        const char PATCH##n[] = content
    #define UNUSED(n)\
        (void)PATCH_LOC##n;\
        (void)PATCH##n;
    
    #include "patches.h" // hand-written c code patches
    #include "gen.h" // generated from patches/
    #ifndef NO_RANDO
    #include "sniff.h" // generated list of sniffing spots
    #include "doggo.h" // generated list of doggo changes
    
    DEF(JUKEBOX_SJUNGLE,      0x938664 - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1
    DEF(JUKEBOX_RAPTORS_1,    0x9391fa - 0x800000, "\x29\x84\x00\x0f\x4d\x4d"); // CALL jukebox3, NOP, NOP
    DEF(JUKEBOX_RATPROS_3,    0x938878 - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1
    DEF(JUKEBOX_DEFEND,       0x94e5b9 - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1
    DEF(JUKEBOX_NJUNGLE,      0x939664 - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1
    DEF(JUKEBOX_EJUNGLE,      0x93b28a - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1
    DEF(JUKEBOX_ECRUSTACIA,   0x95bb46 - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1
    DEF(JUKEBOX_ECRUSTACIAR,  0x95ba0b - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1
    DEF(JUKEBOX_CRUSTACIAFP,  0x97c125 - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1
    DEF(JUKEBOX_NOBILIAF,     0x95d72c - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1
    DEF(JUKEBOX_PALACE,       0x95d43f - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1 // TODO: limit because of wall sounds in past-vigor cutscene
    DEF(JUKEBOX_NOBILIAFP,    0x97c579 - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1
    DEF(JUKEBOX_STRONGHHE,    0x94e625 - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1
    DEF(JUKEBOK_SWAMPPEPPER,  0x94dde6 - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1
    DEF(JUKEBOK_SWAMPSLEEP,   0x94def3 - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1
  //DEF(JUKEBOX_MARKET1,      0x96b80a - 0x800000, "\x29\x70\x00\x0f"); // this requires different code if we really want to randomize this
  //DEF(JUKEBOX_MARKET2,      0x96b80f - 0x800000, "\x29\x70\x00\x0f"); // ^
  //DEF(JUKEBOX_NMARKET1,     0x95cb4e - 0x800000, "\x29\x70\x00\x0f"); // ^
  //DEF(JUKEBOX_NMARKET2,     0x95cb53 - 0x800000, "\x29\x70\x00\x0f"); // ^
  //DEF(JUKEBOX_SQUARE1,      0x95e216 - 0x800000, "\x29\x70\x00\x0f"); // ^
  //DEF(JUKEBOX_SQUARE2,      0x95e21b - 0x800000, "\x29\x70\x00\x0f"); // ^
  //DEF(JUKEBOX_RAPTORS_2,    0x9387b5 - 0x800000, "\x29\x79\x00\x0f\x4d\x4d"); // CALL jukebox2, NOP, NOP // can't change boss music :(
  //DEF(JUKEBOX_PRISON,       0x98b0fa - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1 // raptors glitch out
  //DEF(JUKEBOX_VOLCANO_PATH, 0x93ed69 - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1 // raptors glitch out
  //DEF(JUKEBOX_BBM,          0x93c417 - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1 // bbm bridges glitch out
  //DEF(JUKEBOX_WCRUSTACIA,   0x96bd85 - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1 // exploding rocks
  //DEF(JUKEBOX_EHORACE,      0x96c4da - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1 // exploding rocks
  //DEF(JUKEBOX_PALACEG,      0x96d636 - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1 // doggo fountain sounds
  //DEF(JUKEBOX_FEGATHERING,  0x94c312 - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1 // naming character glitches out
  //DEF(JUKEBOX_WSWAMP,       0x948999 - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1 // frippo sounds and leafpads
  //DEF(JUKEBOX_SWAMP,        0x9492d5 - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1 // frippo sounds and leafpads
  //DEF(JUKEBOX_ACIDRAIN,     0x93af47 - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1 // shopping menu glitches out
  //DEF(JUKEBOX_STRONGHHI,    0x94e7cf - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1 // shopping menu glitches out
  //DEF(JUKEBOX_BLIMPSCAVE,   0x95b377 - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1 // shopping menu glitches out
  //DEF(JUKEBOX_FEVILLAGE,    0x94cea4 - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1 // shopping menu glitches out
  //DEF(JUKEBOX_HALLS_MAIN,   0x9795af - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1 // disabled until tested
  //DEF(JUKEBOX_HALLS_NE,     0x97a381 - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1 // disabled until tested
  //DEF(JUKEBOX_HALLS_NE2,    0x97a16d - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1 // disabled until tested
    #endif
    
    #ifndef NO_RANDO
    uint8_t alchemy[ALCHEMY_COUNT];
    _Static_assert(sizeof(alchemy) == 34, "Bad alchemy count");
    for (uint8_t i=0; i<ALCHEMY_COUNT; i++) alchemy[i] = i;
    struct formula ingredients[ALCHEMY_COUNT];
    // preset to vanilla for logic checking without ingredienizer
    {
        const uint8_t ingredient_types[] = INGREDIENT_TYPES; // includes laser
        for (size_t i=0; i<ALCHEMY_COUNT; i++) {
            ingredients[i].type1 = ingredient_types[2*alchemy_locations[i].id+0];
            ingredients[i].type2 = ingredient_types[2*alchemy_locations[i].id+1];
        }
    }
    uint8_t boss_drops[] = BOSS_DROPS;
    uint16_t gourd_drops[ARRAY_SIZE(gourd_drops_data)];
    for (size_t i=0; i<ARRAY_SIZE(gourd_drops); i++) gourd_drops[i] = (uint16_t)i;
    uint16_t sniff_drops[ARRAY_SIZE(sniffs)];
    for (size_t i=0; i<ARRAY_SIZE(sniff_drops); i++) sniff_drops[i] = sniffs[i].val;
    uint8_t doggo_map[ARRAY_SIZE(doggo_vals)]; // for non-chaos only
    for (size_t i=0; i<ARRAY_SIZE(doggo_map); i++) doggo_map[i] = doggo_vals[i];
    uint8_t doggo_changes[ARRAY_SIZE(doggos)]; // preset to vanilla
    for (size_t i=0; i<ARRAY_SIZE(doggo_changes); i++) doggo_changes[i] = doggos[i].val;
    
    int rollcount=0;
    if (randomized)
        printf("Rolling");
    fflush(stdout);
    do {
        if (rollcount>198) {
            free(buf);
            fclose(fsrc);
            die("\nCould not satifsy logic in 200 tries. Giving up.\n");
        }
        if (rollcount>0) printf(".");
        if ((rollcount+strlen("Rolling"))%79 == 0) printf("\n"); else fflush(stdout); // 79 chars per line
        rollcount++;
        if (alchemizer) {
            shuffle_u8(alchemy, ALCHEMY_COUNT);
            if (difficulty==0 && !ingredienizer) {
                // make sure that one of acid rain, flash or speed
                // is obtainable before thraxx on easy
                uint8_t at = (rand64()%2) ? HARD_BALL_IDX : FLASH_IDX;
                uint8_t spell = rand_u8(0, 2);
                spell = (spell==0) ? FLASH_IDX : (spell==1) ? ACID_RAIN_IDX : SPEED_IDX;
                for (size_t i=0; i<ALCHEMY_COUNT; i++) {
                    if (alchemy[i] == at) { // swap alchemy locations
                        alchemy[i] = alchemy[spell];
                        alchemy[spell] = at;
                        break;
                    }
                }
            }
        }
        if (ingredienizer) {
            const uint8_t min_single_cost = 1;
            const uint8_t max_single_cost = 3;
            const uint8_t max_spell_cost = MIN(4+difficulty, 2*max_single_cost);
            int est_total_cost = 92-6 + difficulty*6; // 92/34 for vanilla
            uint8_t cheap_spell_location = (difficulty==0) ? ((rand64()%2) ? HARD_BALL_IDX : FLASH_IDX) : 0xff;
            
            int cur_total_cost = 0;
            if (chaos) {
                for (uint8_t i=0; i<ALCHEMY_COUNT; i++) {
                    uint8_t type1;
                    uint8_t type2;
                    if (alchemy[i] == cheap_spell_location) {
                        type1 = rand_u8(0, 4);
                        type2 = rand_u8_except(0, 4, type1);
                        type1 = pre_thraxx_ingredients[type1];
                        type2 = pre_thraxx_ingredients[type2];
                    } else {
                        type1 = rand_u8(0, 21);
                        type2 = rand_u8_except(0, 21, type1);
                    }
                    int off = (int)cur_total_cost*100 - (int)est_total_cost*100*i/ALCHEMY_COUNT;
                    uint8_t amount1 = rand_amount(min_single_cost, max_single_cost,  off);
                    if (i==LEVITATE_IDX && type1==DRY_ICE)
                        amount1 = 1; // only allow 1 dry ice for levitate
                    cur_total_cost += amount1;
                    off = (int)cur_total_cost*100 - (int)est_total_cost*(100*i+50)/ALCHEMY_COUNT;
                    uint8_t amount2 = rand_amount(min_single_cost, MIN(max_single_cost,max_spell_cost-amount1), off);
                    if (i==LEVITATE_IDX && type2==DRY_ICE)
                        amount2 = 1; // only allow 1 dry ice for levitate
                    cur_total_cost += amount2;
                    ingredients[i].type1 = type1;
                    ingredients[i].type2 = type2;
                    ingredients[i].amount1 = amount1;
                    ingredients[i].amount2 = amount2;
                }
            } else {
                uint8_t ingredient_types[] = INGREDIENT_TYPES; // includes laser
                if (difficulty==0) { // shuffle original pairs for easy
                    shuffle_u8_pairs(ingredient_types, ARRAY_SIZE(ingredient_types)/2);
                    // make sure cheap_spell_location has only pre_threaxx_ingredients
                    for (size_t i=0; i<ALCHEMY_COUNT; i++) {
                        if (alchemy[i] != cheap_spell_location) continue;
                        if (can_buy_ingredient_pre_thraxx(ingredient_types[i*2+0]) &&
                            can_buy_ingredient_pre_thraxx(ingredient_types[i*2+1])) break;
                        for (size_t j=0; j<ALCHEMY_COUNT; j++) {
                            if (!can_buy_ingredient_pre_thraxx(ingredient_types[j*2+0]) ||
                                !can_buy_ingredient_pre_thraxx(ingredient_types[j*2+1])) continue;
                            // swap ingredients
                            SWAP(ingredient_types[i*2+0],ingredient_types[j*2+0], uint8_t);
                            SWAP(ingredient_types[i*2+1],ingredient_types[j*2+1], uint8_t);
                            break;
                        }
                        break;
                    }
                } else { // shuffle original ingredients
                    shuffle_u8(ingredient_types, ARRAY_SIZE(ingredient_types));
                }
                for (uint8_t i=0; i<ALCHEMY_COUNT; i++) {
                    int off = (int)cur_total_cost*100 - (int)est_total_cost*100*i/ALCHEMY_COUNT;
                    uint8_t amount1 = rand_amount(min_single_cost, max_single_cost, off);
                    if (i==LEVITATE_IDX && ingredient_types[i*2] == DRY_ICE)
                        amount1 = 1; // only allow 1 dry ice for levitate
                    cur_total_cost += amount1;
                    off = (int)cur_total_cost*100 - (int)est_total_cost*(100*i+50)/ALCHEMY_COUNT;
                    uint8_t amount2 = rand_amount(min_single_cost, MIN(max_single_cost,max_spell_cost-amount1), off);
                    if (i==LEVITATE_IDX && ingredient_types[i*2+1] == DRY_ICE)
                        amount2 = 1; // only allow 1 dry ice for levitate
                    cur_total_cost += amount2;
                    ingredients[i].type1 = ingredient_types[i*2];
                    ingredients[i].type2 = ingredient_types[i*2+1];
                    ingredients[i].amount1 = amount1;
                    ingredients[i].amount2 = amount2;
                }
            }
        }
        if (bossdropamizer) {
            shuffle_u8(boss_drops, ARRAY_SIZE(boss_drops));
        }
        if (gourdomizer) {
            shuffle_u16(gourd_drops, ARRAY_SIZE(gourd_drops));
            // make sure amulet of annihilation is available in ivor tower
            {
                uint8_t amuletNo = rand_u8(0,3);
                uint8_t ivorGourdNo = rand_u8(0,23);
                size_t amuletSrcIdx=(size_t)-1, ivorGourdIdx=(size_t)-1;
                for (size_t i=0; i<ARRAY_SIZE(gourd_drops); i++) {
                    if (stris(gourd_data[i].name, "Ivor Houses")) {
                        if (ivorGourdNo == 0) {
                            ivorGourdIdx = i;
                            if (amuletSrcIdx != (size_t)-1) break;
                        }
                        ivorGourdNo--;
                    }
                    if (stris(gourd_drops_data[gourd_drops[i]].name, "Amulet of Annihilation")) {
                        if (amuletNo == 0) {
                            amuletSrcIdx = i;
                            if (ivorGourdIdx != (size_t)-1) break;
                        }
                        amuletNo--;
                    }
                }
                assert(amuletSrcIdx<ARRAY_SIZE(gourd_drops) && ivorGourdIdx<ARRAY_SIZE(gourd_drops));
                SWAP(gourd_drops[ivorGourdIdx],gourd_drops[amuletSrcIdx],uint16_t);
            }
            // make sure wings are available in halls NE room
            {
                uint8_t wingsNo = rand_u8(0,4);
                uint8_t hallsNEGourdNo = rand_u8(0,3);
                size_t wingsSrcIdx=(size_t)-1, hallsNEGourdIdx=(size_t)-1;
                for (size_t i=0; i<ARRAY_SIZE(gourd_drops); i++) {
                    if (stris(gourd_data[i].name, "Halls NE")) {
                        if (hallsNEGourdNo == 0) {
                            hallsNEGourdIdx = i;
                            if (wingsSrcIdx != (size_t)-1) break;
                        }
                        hallsNEGourdNo--;
                    }
                    if (stris(gourd_drops_data[gourd_drops[i]].name, "Wings")) {
                        if (wingsNo == 0) {
                            wingsSrcIdx = i;
                            if (hallsNEGourdIdx != (size_t)-1) break;
                        }
                        wingsNo--;
                    }
                }
                assert(wingsSrcIdx<ARRAY_SIZE(gourd_drops) && hallsNEGourdIdx<ARRAY_SIZE(gourd_drops));
                SWAP(gourd_drops[hallsNEGourdIdx],gourd_drops[wingsSrcIdx],uint16_t);
            }
        }
        if (doggomizer && !chaos) {
            shuffle_u8(doggo_map+1, ARRAY_SIZE(doggo_map)-2); // keep act0 and act4 dog
            for (size_t i=0; i<ARRAY_SIZE(doggo_changes); i++) {
                for (size_t j=0; j<ARRAY_SIZE(doggo_map); j++) {
                    if (doggo_changes[i] != doggo_vals[j]) continue;
                    doggo_changes[i] = doggo_map[j];
                    break;
                }
            }
        } else if (doggomizer) {
            for (size_t i=0; i<ARRAY_SIZE(doggo_changes); i++) {
                if (doggo_changes[i] == DOGGO_ACT4) continue; // don't touch act4 spots
                doggo_changes[i] = doggo_vals[rand_u8(0, ARRAY_SIZE(doggo_vals)-2)]; // act0-3 dogs only
            }
        }
        if (sniffamizer && !chaos) {
            shuffle_u16(sniff_drops, ARRAY_SIZE(sniff_drops));
        } else if (sniffamizer) {
            for (size_t i=0; i<ARRAY_SIZE(sniff_drops); i++) {
                sniff_drops[i] = rand_u16(0x0200, 0x0200+ARRAY_SIZE(ingredient_names)-1);
            }
        }
        
        // general logic checking
        #define REROLL() continue;
        
        // boss drop logic: thraxx has to drop a weapon unless gourdomizer is on. we can get back from thraxx to bug muck
        if (bossdropamizer && !gourdomizer)
        {
            if (!boss_drop_is_a_weapon(boss_drops[THRAXX_IDX])) REROLL();
        }
        
        struct formula* levitate_formula = &ingredients[LEVITATE_IDX];
        struct formula* revealer_formula = &ingredients[REVEALER_IDX];
        struct formula* atlas_formula = &ingredients[ATLAS_IDX];
        {
            if (levitate_formula->type1 == METEORITE ||
                levitate_formula->type2 == METEORITE)
                    REROLL(); // reroll, unbeatable or would give away a hint
            if ((levitate_formula->type1 == DRY_ICE && levitate_formula->amount1>1) ||
                (levitate_formula->type2 == DRY_ICE && levitate_formula->amount2>1))
                    REROLL();
            if ((levitate_formula->type1 == DRY_ICE && levitate_formula->amount1==1) ||
                (levitate_formula->type2 == DRY_ICE && levitate_formula->amount2==1))
            {
                // no other formula may use dry ice
                bool ok = true;
                for (size_t i=0; i<ALCHEMY_COUNT; i++) {
                    if (i != LEVITATE_IDX && (ingredients[i].type1 == DRY_ICE || ingredients[i].type2 == DRY_ICE)) {
                        ok = false;
                        break;
                    }
                }
                if (!ok) REROLL();
            }
            if (!can_buy_ingredients(revealer_formula)) REROLL(); // reroll, unbeatable or would give away a hint
            if (alchemy_missable(alchemy[LEVITATE_IDX])) REROLL(); // NOTE: alchemy[a] = b moves a to vanilla b location
            if (alchemy_missable(alchemy[REVEALER_IDX])) REROLL(); // reroll, unbeatable or would give away a hint
            if (!alchemy_in_act4(alchemy[LEVITATE_IDX]) && !can_buy_in_act3(levitate_formula)) REROLL(); // as above
            if (!alchemy_in_act4(alchemy[REVEALER_IDX]) && !can_buy_in_act3(revealer_formula)) REROLL(); // as above
            // make sure atlas can be cast on easy in act1-3
            if (difficulty == 0) { // easy
                if (gourdomizer) {
                     if (atlas_formula->type1 == GREASE || atlas_formula->type2 == GREASE) REROLL();
                     if (atlas_formula->type1 == DRY_ICE || atlas_formula->type2 == DRY_ICE) REROLL();
                     if (atlas_formula->type1 == METEORITE || atlas_formula->type2 == METEORITE) REROLL();
                     if (atlas_formula->type1 == GUNPOWDER || atlas_formula->type2 == GUNPOWDER) REROLL();
                } else {
                     if (atlas_formula->type1 == GREASE && atlas_formula->amount1>1) REROLL();
                     if (atlas_formula->type2 == GREASE && atlas_formula->amount2>1) REROLL();
                     if (atlas_formula->type1 == DRY_ICE && atlas_formula->amount1>2) REROLL();
                     if (atlas_formula->type2 == DRY_ICE && atlas_formula->amount2>2) REROLL();
                     if (atlas_formula->type1 == METEORITE && atlas_formula->amount1>2) REROLL();
                     if (atlas_formula->type2 == METEORITE && atlas_formula->amount2>2) REROLL();
                     if (atlas_formula->type1 == GUNPOWDER && atlas_formula->amount1>2) REROLL();
                     if (atlas_formula->type2 == GUNPOWDER && atlas_formula->amount2>2) REROLL();
                }
            }
            // make sure we get one castable spell pre-thraxx on easy
            // NOTE: this should be guaranteed by generation
            if ((ingredienizer || alchemizer) && difficulty == 0) {
                uint8_t castable = 2;
                for (size_t i=0; i<ALCHEMY_COUNT; i++)
                    if ((alchemy[i]==FLASH_IDX || alchemy[i]==HARD_BALL_IDX) &&
                            !can_buy_pre_thraxx(&ingredients[i]))
                        castable--;
                if (castable<1) {
                    fprintf(stderr, "PRE-THRAXX INGREDIENTS BUG: "
                            "Please report this with seed and settings.\n");
                    REROLL();
                }
            }
            // make sure no spell uses the same ingredient twice
            if (ingredienizer) {
                bool ok = true;
                for (uint8_t i=0; i<ALCHEMY_COUNT; i++)
                    if (ingredients[i].type1 == ingredients[i].type2)
                        ok = false;
                if (!ok) REROLL();
            }
        }
        #undef REROLL
        // tree-based milestone logic checking
        bool reroll = false;
        #define REROLL() { reroll = true; break; }
        for (int milestone=0; milestone<2; milestone++)
        {
            enum progression goal = milestone==0 ? P_ROCKET : P_ENERGY_CORE;
            bool allow_rockskip=!fixsequence && !glitchless;
            bool allow_saturnskip=!fixsequence && !glitchless;
            check_tree_item checks[ARRAY_SIZE(blank_check_tree)];
            memcpy(checks, blank_check_tree, sizeof(blank_check_tree));
            int progress[P_END]; memset(progress, 0, sizeof(progress));
            if (allow_rockskip) progress[P_ROCK_SKIP]++;
            bool complete=false;
            while (!complete) {
                complete = true;
                for (size_t i=0; i<ARRAY_SIZE(checks); i++) {
                    if (checks[i].reached) continue;
                    if (check_requires(checks+i, goal)) continue; // don't iterate past goal
                    if (check_reached(checks+i, progress)) {
                        // NOTE: alchemy[a] = b moves a to vanilla b location
                        uint16_t idx = checks[i].type==CHECK_ALCHEMY ? alchemy_lookup(alchemy,checks[i].index) :
                                       checks[i].type==CHECK_BOSS ? boss_drops[checks[i].index] :
                                       checks[i].type==CHECK_GOURD ? gourd_drops[checks[i].index] : 0;
                        const drop_tree_item* drop = get_drop(checks[i].type, idx);
                        check_progress(checks+i, progress);
                        if (! checks[i].missable)
                            drop_progress(drop, progress);
                        #ifdef DEBUG_CHECK_TREE
                        printf("Reached %s\n", check2str(checks+i));
                        if (drop) printf("Got %s%s\n", checks[i].missable?"missable ":"", drop2str(drop));
                        #endif
                        complete=false;
                    }
                }
            }
            if (allow_saturnskip && goal==P_ENERGY_CORE) { /* goal optional */ }
            else if (progress[goal]<1) REROLL();
            // make sure atlas is reachable if it should be
            if (difficulty==milestone && progress[P_ATLAS]<1) REROLL();
            // FIXME: add ingredients to check-table, so we don't have to do this?
            // NOTE: ingredient requirements always active now and moved to pre-check
        }
        #undef REROLL
        if (reroll) continue;
        break;
    } while (true);
    if (randomized) printf("\n");
    #endif
    
    // apply patches
    #define APPLY(n) APPLY_PATCH(buf, PATCH##n, rom_off + PATCH_LOC##n)
    
    if (fixsequence && !openworld) {
        free(buf);
        fclose(fsrc);
        die("Cannot fix glitches without applying open world patch-set!\n");
    }
    
    #ifndef NO_RANDO
    if (bossdropamizer && !openworld) {
        free(buf);
        fclose(fsrc);
        die("Cannot randomize boss drops without open world patch-set (yet)!\n");
    }
    #endif
    
    if (openworld) {
        printf("Applying open world patch-set...\n");
        APPLY(1);  APPLY(2);  APPLY(3);  APPLY(4);  APPLY(5);  APPLY(6);
        APPLY(7);  APPLY(8);  APPLY(9);  APPLY(10); APPLY(11); APPLY(12);
        APPLY(13); APPLY(14); APPLY(15); APPLY(16); APPLY(17); APPLY(18);
        APPLY(19); APPLY(20); APPLY(21); APPLY(22); APPLY(23); APPLY(24);
        APPLY(25); APPLY(26); APPLY(27); APPLY(28); APPLY(29); APPLY(30);
        APPLY(31); /* -32- */ APPLY(33); APPLY(34); APPLY(35); APPLY(36);
        APPLY(37); APPLY(38);
        // v005:
        APPLY(39); APPLY(40); APPLY(41); APPLY(42); APPLY(43); APPLY(44);
        APPLY(45); APPLY(46); APPLY(47); /*48-49: see below*/  APPLY(50);
        APPLY(51); APPLY(52); APPLY(53); APPLY(54); APPLY(55); APPLY(56);
        APPLY(57); APPLY(58); APPLY(59);
        // v006:
        APPLY(60); APPLY(61); APPLY(62); APPLY(63); APPLY(64); APPLY(65);
        // v008:
        APPLY(66); /*67-68: see below*/  APPLY(69);
        // v026: travel forth and back through oglin cave
        APPLY(OGLIN_CAVE);  APPLY(OGLIN_CAVE2); APPLY(OGLIN_CAVE3);
        APPLY(OGLIN_CAVE4); APPLY(OGLIN_CAVE5); APPLY(OGLIN_CAVE6);
        // v009:
        APPLY(72); /*73,74: see below*/  APPLY(75); APPLY(76);
        // v015:
        APPLY(139); APPLY(140); /* supersedes 32 */
        // v017:
        APPLY(147); APPLY(149a); APPLY(149b);
        // v018:
        APPLY(FE_VILLAGE_WW);   APPLY(FE_VILLAGE_WW2);  APPLY(FE_VILLAGE_WW3);
        APPLY(FE_VILLAGE_WW4);  APPLY(FE_VILLAGE_WW5);  APPLY(FE_VILLAGE_WW6);
        APPLY(FE_VILLAGE_WW7);  APPLY(FE_VILLAGE_WW8);  APPLY(FE_VILLAGE_WW9);
        APPLY(FE_VILLAGE_WW10); APPLY(FE_VILLAGE_WW11); APPLY(FE_VILLAGE_WW12);
        APPLY(FE_VILLAGE_WW13); APPLY(FE_VILLAGE_WW14); APPLY(FE_VILLAGE_WW15);
        APPLY(FE_VILLAGE_WW16); APPLY(FE_VILLAGE_WW17); APPLY(FE_VILLAGE_WW18);
        APPLY(FE_VILLAGE_WW19);
        APPLY(MARKET_REWORK);
        APPLY(ACT3_OW);  APPLY(ACT3_OW2); APPLY(ACT3_OW3);
        APPLY(MUD_PEPPER_LIMIT);
        APPLY(TEMP); APPLY(TEMP2); // TODO: resolve underlying issue
        // v026: levitate last rock entering magmar fight to avoid softlock
        APPLY(VOLCANO);
    }
    if (keepdog) {
        printf("Applying patches to keep the dog...\n");
        APPLY(KEEP_DOG); APPLY(KEEP_DOG2);
    }
    
    // General bug fixes
    printf("Fixing vanilla softlocks...\n");
    // v009:
    APPLY(73);
    // v015:
    APPLY(141); APPLY(142);
    // v017:
    APPLY(146);
    APPLY(148);
    // v025:
    APPLY(HALLS_BUGS); // (actually not a softlock but a bug)
    // v026: fix colosseum entrance with dog available
    APPLY(COLOSSEUM7);APPLY(COLOSSEUM8);
    APPLY(COLOSSEUM9);APPLY(COLOSSEUM10);
    
    printf("Fixing some missables...\n");
    // v018:
    APPLY(ESCAPE_AFTER_DES); // would only be required for alchemizer
    APPLY(ESCAPE_AFTER_DES2); // TODO: merge this with horace spawn changes
    APPLY(ESCAPE_AFTER_DES3); // 2 and 3 is required for changed horace spawn
    APPLY(PALACE_REWORK); // would only be required for alchemizer
    APPLY(QUEENS_KEY);
    APPLY(QUEENS_KEY2);
    APPLY(QUEENS_KEY3);
    APPLY(QUEENS_KEY4);
    APPLY(QUEENS_KEY5);
    // v023:
    APPLY(QUEENS_KEY6);
    // v026:
    APPLY(QUEENS_KEY7);
    // v020:
    APPLY(FIX_FLAGS);
    // v023:
    APPLY(FIX_FLAGS2); APPLY(FIX_FLAGS3);
    
    #ifndef NO_RANDO
    if (gourdomizer || bossdropamizer) {
        // v0015:
        APPLY(126); APPLY(127); APPLY(128); APPLY(129); APPLY(130); APPLY(131);
        APPLY(132); APPLY(133); APPLY(134); APPLY(135); APPLY(136); APPLY(137);
        APPLY(138); 
    }
    #endif
    // General features
    printf("Improving quality of life...\n");
    // v014:
    APPLY(106);
    
    if (fixsequence) {
        printf("Applying desolarify patch-set...\n");
        APPLY(48);
        APPLY(49);
        printf("Applying desaturnate patch...\n");
        APPLY(68);
        printf("Disabling double gauge...\n");
        APPLY(145);
    }
    
    if (fixcheats) { // excluding atlas
        printf("Removing infinite call bead glitch...\n");
        APPLY(67);
    }
    
    #ifdef NO_RANDO // get rid of unused warnings
    UNUSED(74);
    UNUSED(77);
    #else
    if (alchemizer) {
        printf("Applying alchemizer...\n");
        // Alchemy preselection relocation
        grow = true;
        APPLY(107);
        APPLY(108); APPLY(109); APPLY(110); APPLY(111); APPLY(112); APPLY(113);
        APPLY(114); APPLY(115); APPLY(116); APPLY(117); APPLY(118); APPLY(119);
        APPLY(120); APPLY(121); APPLY(122); APPLY(123); APPLY(124); APPLY(125);
        // Write randomized values
        for (uint8_t i=0; i<ALCHEMY_COUNT; i++) {
            // write value from i to locateions of alchemy[i]
            uint16_t id = alchemy_locations[i].id;
            size_t to = alchemy[i];
            //printf("%s @ %s\n", alchemy_locations[i].name, alchemy_locations[to].name);
            for (size_t j=0; alchemy_locations[to].locations[j] != LOC_END; j++) {
                size_t loc = alchemy_locations[to].locations[j];
                assert(loc & 0x8000);
                buf[rom_off + loc + 0] = id & 0xff;
                buf[rom_off + loc + 1] = id >> 8;
                //printf("  writing 0x%02x to 0x%06x\n", (unsigned)id, (unsigned)loc);
            }
            for (size_t j=0; j<ARRAY_SIZE(alchemy_locations[to].preselects); j++) {
                size_t loc = alchemy_locations[to].preselects[j];
                if (loc == LOC_END) continue; // or break
                assert(loc & 0x8000);
                buf[rom_off + loc] = id<<1;
            }
        }
    }
    if (ingredienizer) {
        _Static_assert(sizeof(*ingredients)==4, "Bad padding"); // required for memcpy
        printf("Applying ingredienizer...\n");
        for (uint8_t i=0; i<ALCHEMY_COUNT; i++) {
            uint16_t id = alchemy_locations[i].id;
            memcpy(buf + rom_off + 0x4601F + id*4, &(ingredients[i]), 4);
        }
    }
    if (bossdropamizer) {
        printf("Applying fixes for randomized boss drops...\n");
        APPLY(74); APPLY(78); APPLY(79); APPLY(80);
        // v015:
        APPLY(78a); APPLY(78b); APPLY(78c); APPLY(78d); APPLY(78e);
        APPLY(143); APPLY(144);
        if (!gourdomizer) {
            // v026: swap Halls SW Wings with Halls NE Wax
            APPLY(HALLS_WINGS);  APPLY(HALLS_WINGS2);
            APPLY(HALLS_WINGS3); APPLY(HALLS_WINGS4);
            APPLY(HALLS_WINGS5); APPLY(HALLS_WINGS6);
            APPLY(HALLS_WINGS7); APPLY(HALLS_WINGS8);
        }
        printf("Applying boss dropamizer...\n");
        APPLY(77);
        APPLY(81);  APPLY(82);  APPLY(83);  APPLY(84);  APPLY(85);  APPLY(86);
        APPLY(87);  APPLY(88);  APPLY(89);  APPLY(90);  APPLY(91);  APPLY(92);
        APPLY(93);  APPLY(94);  APPLY(95);  APPLY(96);  APPLY(97);  APPLY(98);
        APPLY(99);  APPLY(100); APPLY(101); APPLY(102); APPLY(103); APPLY(104);
        APPLY(105);
        // actually apply boss drop randomization
        for (size_t i=0; i<ARRAY_SIZE(boss_drops); i++) {
            uint32_t tgt = boss_drop_jump_targets[boss_drops[i]];
            buf[rom_off + boss_drop_jumps[i] + 0] = (uint8_t)(tgt>>0)&0xff;
            buf[rom_off + boss_drop_jumps[i] + 1] = (uint8_t)(tgt>>8)&0xff;
            buf[rom_off + boss_drop_jumps[i] + 2] = (uint8_t)(tgt>>16)&0xff;
        }
    }
    if (gourdomizer) {
        // v023:
        printf("Applying fixes for randomized gourds...\n");
        APPLY(REVERSE_BBM); APPLY(REVERSE_BBM2); APPLY(REVERSE_BBM3);
        printf("Applying gourdomizer...\n");
        grow = true;
        APPLY(GOURDOMIZER_FIXES); APPLY(GOURDOMIZER_FIXES2);
        APPLY(GOURDOMIZER_FIXES3);
        APPLY(GOURDOMIZER_DROPS);  APPLY(GOURDOMIZER_DROPS2);
        APPLY(GOURDOMIZER_DROPS3); APPLY(GOURDOMIZER_DROPS4);
        APPLY(GOURDOMIZER_DROPS5); APPLY(GOURDOMIZER_DROPS6);
        APPLY(GOURDOMIZER_DROPS7);
        // v026: re-enter colosseum
        APPLY(COLOSSEUM);  APPLY(COLOSSEUM2);
        APPLY(COLOSSEUM3); APPLY(COLOSSEUM4);
        APPLY(COLOSSEUM5); APPLY(COLOSSEUM6);
        for (size_t i=0; i<ARRAY_SIZE(gourd_drops_data); i++) {
            const struct gourd_drop_item* d = &(gourd_drops_data[i]);
            memcpy(buf + rom_off + d->pos, d->data, d->len);
        }
        for (size_t i=0; i<ARRAY_SIZE(gourd_data); i++) {
            const struct gourd_data_item* g = &(gourd_data[i]);
            const struct gourd_drop_item* d = &(gourd_drops_data[gourd_drops[i]]);
            memcpy(buf + rom_off + g->pos, g->data, g->len);
            buf[rom_off + g->call_target_addr+0] = (d->call_addr>> 0) & 0xff;
            buf[rom_off + g->call_target_addr+1] = (d->call_addr>> 8) & 0xff;
            buf[rom_off + g->call_target_addr+2] = (d->call_addr>>16) & 0xff;
        }
    }
    if (musicmizer) {
        printf("Applying musicmizer...\n");
        // NOTE: this is actually for jukebox, not musicmizer
        grow = true;
        APPLY(JUKEBOX);
        APPLY(JUKEBOX_SJUNGLE);
        APPLY(JUKEBOX_RAPTORS_1);
        APPLY(JUKEBOX_RATPROS_3);
        APPLY(JUKEBOX_NJUNGLE);
        APPLY(JUKEBOX_EJUNGLE);
        APPLY(JUKEBOX_DEFEND);
        APPLY(JUKEBOX_ECRUSTACIA);
        APPLY(JUKEBOX_ECRUSTACIAR);
        APPLY(JUKEBOX_CRUSTACIAFP);
        APPLY(JUKEBOX_NOBILIAF);
        APPLY(JUKEBOX_PALACE);
        APPLY(JUKEBOX_NOBILIAFP);
        APPLY(JUKEBOX_STRONGHHE);
        APPLY(JUKEBOK_SWAMPPEPPER);
        APPLY(JUKEBOK_SWAMPSLEEP);
    }
    if (sniffamizer) {
        printf("Applying sniffamizer...\n");
        if (!openworld)
            APPLY(MUD_PEPPER_LIMIT);
        for (size_t i=0; i<ARRAY_SIZE(sniff_drops); i++) {
            buf[rom_off + sniffs[i].addr + 0] = (uint8_t)(sniff_drops[i]&0xff);
            buf[rom_off + sniffs[i].addr + 1] = (uint8_t)(sniff_drops[i]>>8);
        }
    }
    if (doggomizer) {
        printf("Applying doggomizer...\n");
        for (size_t i=0; i<ARRAY_SIZE(doggo_changes); i++) {
            if (doggos[i].hard && difficulty<2) continue; // skip
            buf[rom_off + doggos[i].addr] = doggo_changes[i];
        }
    }
    
    // if check value differs, the generated ROMs are different.
    uint32_t seedcheck = (uint16_t)(rand64()&0x3ff); // 10bits=2 b32 chars
    if (openworld)      seedcheck |= 0x00000400;
    if (fixsequence)    seedcheck |= 0x00000800;
    if (fixcheats)      seedcheck |= 0x00001000; // excluding atlas
    if (glitchless)     seedcheck |= 0x00002000;
    if (bossdropamizer) seedcheck |= 0x00004000;
    if (alchemizer)     seedcheck |= 0x00008000;
    if (ingredienizer)  seedcheck |= 0x00010000;
    if (gourdomizer)    seedcheck |= 0x00020000;
    if (sniffamizer)    seedcheck |= 0x00040000;
    if (doggomizer)     seedcheck |= 0x00080000;
  //if (enemizer)       seedcheck |= 0x00100000;
    if (keepdog)        seedcheck |= 0x00200000;
    // 0x00400000 and 0x00800000 = difficulty
    if (chaos)          seedcheck |= 0x01000000; // 25bits in use -> 5 b32 chars
    seedcheck |= ((uint32_t)difficulty<<22);
    printf("\nCheck: %c%c%c%c%c (Please compare before racing)\n",
           b32(seedcheck>>20), b32(seedcheck>>15),
           b32(seedcheck>>10), b32(seedcheck>>5),  b32(seedcheck>>0));
    #endif
    
    
    char shortsettings[sizeof(settings)];
    {
        memset(shortsettings, 0, sizeof(shortsettings));
        char* a = shortsettings; char* b = settings;
        while (*b) if (*b!='r' && *b!='l') *a++=*b++; else b++;
        if (!shortsettings[0]) shortsettings[0]='r';
    }
#ifdef NO_RANDO
    char dsttitle[strlen("SoE-OpenWorld_")+strlen(VERSION)+1+sizeof(shortsettings)-1+1]; // SoE-OpenWorld_vXXX_e0123
#else
    char dsttitle[strlen("Evermizer_")+strlen(VERSION)+1+sizeof(shortsettings)-1+1+16+1]; // Evermizer_vXXX_e0123caibgsdm_XXXXXXXXXXXXXXXX
    assert(snprintf(dsttitle, sizeof(dsttitle), "Evermizer_%s_%s_%" PRIx64, VERSION, shortsettings, seed)<sizeof(dsttitle));
    if (!randomized)
#endif
        assert(snprintf(dsttitle, sizeof(dsttitle), "SoE-OpenWorld_%s_%s", VERSION, shortsettings)<sizeof(dsttitle));
    char* pSlash = strrchr(src, DIRSEP);
    if (!pSlash && DIRSEP!='/') pSlash = strrchr(src, '/'); // wine support
    const char* ext = strrchr(src, '.');
    if (!ext || ext<pSlash) ext = ".sfc";
    size_t baselen = pSlash ? (pSlash-src+1) : 0;
    char dstbuf[dstdir? (strlen(dstdir)+1+strlen(dsttitle)+strlen(ext)) : (baselen+strlen(dsttitle)+strlen(ext))+1];
    if (dstdir) {
        size_t p = strlen(dstdir);
        memcpy(dstbuf, dstdir, p);
        if (p>0 && dstbuf[p-1]!='/' && dstbuf[p-1]!='\\')
            dstbuf[p++]=DIRSEP;
        memcpy(dstbuf+p, dsttitle, strlen(dsttitle));
        memcpy(dstbuf+p+strlen(dsttitle), ext, strlen(ext)+1);
    } else {
        if (baselen) memcpy(dstbuf, src, baselen);
        char* p = dstbuf + (pSlash ? (pSlash-src+1) : 0);
        memcpy(p, dsttitle, strlen(dsttitle));
        memcpy(p+strlen(dsttitle), ext, strlen(ext)+1);
    }
    
    const char* dst = (ofn && *ofn) ? ofn : dstbuf;
    
    FILE* fdst = fopen(dst,"wb");
    if (!fdst) { fclose(fsrc); free(buf); die("Could not open output file!\n"); }
    if (grow) sz+=GROW_BY;
    // TODO: recalculate checksum
    len = fwrite(buf, 1, sz, fdst);
    fclose(fdst); fdst=NULL;
    if (len<sz) { fclose(fsrc); free(buf); die("Could not write output file!\n"); }
    printf("Rom saved as %s!\n", dst);
    
    // write spoiler log
#ifndef NO_RANDO
    if (spoilerlog) {
    char logdstbuf[strlen(dst)+strlen("_SPOILER.log")+1];
    pSlash = strrchr(dst, DIRSEP);
    ext = strrchr(dst, '.');
    if (!ext || ext<pSlash) ext = dst+strlen(dst);
    memcpy(logdstbuf, dst, ext-dst);
    memcpy(logdstbuf+(ext-dst), "_SPOILER.log", strlen("_SPOILER.log")+1);
    
    FILE* flog = fopen(logdstbuf,"wb");
    if (!flog) { fclose(fsrc); free(buf); die("Could not open spoiler log file!\n"); }
    #define ENDL "\r\n"
    fprintf(flog,"Spoiler log for evermizer %s settings %s seed %" PRIx64 "%s", VERSION, shortsettings, seed, ENDL);
    fprintf(flog, ENDL);
    fprintf(flog,"     %-15s  %-15s  %-15s   %s" ENDL, "Spell", "Location", "Ingredient 1", "Ingredient 2"); 
    fprintf(flog,"------------------------------------------------------------------------" ENDL);
    for (size_t i=0; i<ALCHEMY_COUNT; i++) {
        struct formula* f = &(ingredients[i]);
        size_t to = alchemy[i];
        
        fprintf(flog,"(%02d) %-15s  %-15s  %dx %-12s + %dx %s" ENDL,
            alchemy_locations[i].id, 
            alchemy_locations[i].name,
            alchemy_locations[to].name,
            f->amount1, ingredient_names[f->type1],
            f->amount2, ingredient_names[f->type2]);
    }
    fprintf(flog,"------------------------------------------------------------------------" ENDL);
    fprintf(flog, ENDL);
    fprintf(flog,"     %-13s  %s" ENDL, "Boss", "Drop");
    fprintf(flog,"------------------------------------------------------------------------" ENDL);
    for (size_t i=0; i<ARRAY_SIZE(boss_drops); i++) {
        fprintf(flog,"(%02d) %-13s  %s" ENDL, (int)i, boss_names[i], boss_drop_names[boss_drops[i]]);
    }
    fprintf(flog,"------------------------------------------------------------------------" ENDL);
    if (gourdomizer) {
    fprintf(flog, ENDL);
    fprintf(flog,"      %-19s  %s" ENDL, "Gourd", "Drop");
    fprintf(flog,"------------------------------------------------------------------------" ENDL);
    for (size_t i=0; i<ARRAY_SIZE(gourd_drops); i++) {
        size_t j=gourd_drops[i];
        if (!gourd_drops_data[j].name || !gourd_drops_data[j].name[0]) continue;
        fprintf(flog,"(%03d) %-19s  %s" ENDL, (int)i, gourd_data[i].name, gourd_drops_data[j].name);
    }
    fprintf(flog,"------------------------------------------------------------------------" ENDL);
    }
    #undef ENDL
    fclose(flog); flog=NULL;
    printf("Spoiler log saved as %s!\n", logdstbuf);
    }
#endif

    free(buf);
    fclose(fsrc);
#if (defined(WIN32) || defined(_WIN32)) && !defined(NO_UI)
    if (!batch) system("pause");
#endif
}
