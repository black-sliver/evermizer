// single-c-file is the simplest way to get a static EXE and WASM
// from a common source, also without requiring project/make files and LTO
// see google doc for documentation on old (numbered) patches
// the idea is to "manually" patch the game to a state where we simply swap
// out some numbers to make it random (without rewriting/relocating everything)
#define VERSION "v050.1"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#ifdef _MSC_VER
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#else
#include <strings.h> // strcasecmp
#endif
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h> 
#include "tinymt64.h"
#ifndef NO_UI // includes and helpers for UI
#if defined(WIN32) || defined(_WIN32)
#include <process.h>
#include <windows.h>
int getch(void); // can't #include <conio.h> with <windows.h>
void clrscr(void)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD pos = {0,0};
    int i;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    for (i=0; i<csbi.srWindow.Bottom-csbi.srWindow.Top+1+3; i++)
        printf("\n");
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}
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
#define pause() do{printf("\nPress any key to quit ...\n"); getch();}while(false)

#if defined(WIN32) || defined(_WIN32)
#define DIRSEP '\\'
#ifndef PRIx64
#define PRIx64 "I64x"
#define uint64_t unsigned __int64
#endif
#else
#define DIRSEP '/'
#endif

#if !defined NO_MULTIWORLD && !defined WITH_MULTIWORLD && !defined __EMSCRIPTEN__
#define WITH_MULTIWORLD
#endif

#include "util.h"

static bool stris(const char* s, const char* t) { if (s==t) return true; return strcmp(s,t)==0; }

#ifdef NO_UI
static bool batch = true;
#else
static bool batch = false;
#endif
#ifndef die
void die(const char* msg)
{
    if (msg) fprintf(stderr, "%s", msg);
#if (defined(WIN32) || defined(_WIN32)) && !defined(NO_UI)
    if (!batch) pause();
#else
    (void)batch; // ignore warnings
#endif
    exit(1);
}
#endif

const char B32[] = "abcdefghijklmnopqrstuvwxyz234567=";
static char b32(uint64_t v) { return B32[v&0x1f]; }

#define APPLY_PATCH(buf, patch, loc) memcpy(buf+loc, patch, sizeof(patch)-1)

#include "rng.h"

#define progressive_armor (true) // NOTE: gourdomizer only supports true
#include "gourds.h" // generated list of gourds and gourd drops
#include "sniff.h" // generated list of sniffing spots
#include "data.h"


#define float DO_NOT_USE_FLOATS
#define double DO_NOT_USE_FLOATS


// Misc consts
const char DIFFICULTY_CHAR[] = {'e','n','h','x'};
const char* const DIFFICULTY_NAME[] = {"Easy","Normal","Hard","Random"};
const char* OFF_ON[] = { "Off", "On", NULL };
const char* OFF_ON_FULL[] = { "Off", "On", "Full", NULL };
const char* OFF_ON_POOL[] = { "Off", "On", "Pool", NULL };
const char* OFF_ON_LOGIC[] = { "Off", "On", "In Logic", NULL };
const char* VANILLA_RANDOM[] = { "Vanilla", "Random", NULL };
const char* ENERGY_CORE_VALUES[] = { "Vanilla", "Shuffle", "Fragments", NULL };
const char* POOL_STRATEGY_VALUES[] = { "Balance", "Random", "Bosses", NULL };
#define ON 1
#define FULL 2
#define POOL 2
#define LOGIC 2
#define STRATEGY_BALANCED 0
#define STRATEGY_RANDOM 1
#define STRATEGY_BOSSES 2
#define ENERGY_CORE_VANILLA 0
#define ENERGY_CORE_SHUFFLE 1
#define ENERGY_CORE_FRAGMENTS 2

#define DEFAULT_difficulty 1
struct option { char key; uint8_t def; const char* text; const char* info; const char* description; const char** state_names; const char* section; const char* subsection;};
const static struct option options[] = {
    { 0,   1, "Open World", NULL,          "Make windwalker available in every firepit", OFF_ON, NULL, NULL },    
    { '4', 0, "All accessible", NULL,      "Make sure all key items are obtainable", OFF_ON, "General", NULL },
    { 'l', 0, "Spoiler Log", NULL,         "Generate a spoiler log file", OFF_ON, "General", NULL },
    { 'z', 1, "Energy Core", NULL,         "How to obtain the Energy Core. Random and Fragments convert the vanilla spot to a gourd. "
                                           "Fragments replace ingredients in gourds.", ENERGY_CORE_VALUES, "General", "Key items" },
    { 'a', 1, "Alchemizer", NULL,          "Shuffle learned alchemy formulas. Select 'pool' to add this pool to the mixed pool.", OFF_ON_POOL, "General", "Key items" },
    { 'b', 1, "Boss dropamizer", NULL,     "Shuffle boss drops. Select 'pool' to add this pool to the mixed pool.", OFF_ON_POOL, "General", "Key items" },
    { 'g', 1, "Gourdomizer", NULL,         "Shuffle gourd drops. Select 'pool' to add this pool to the mixed pool.", OFF_ON_POOL, "General", "Key items" },
    { 's', 1, "Sniffamizer", NULL,         "Shuffle sniff drops. Select 'pool' to add this pool to the mixed pool.", OFF_ON_POOL, "General", "Key items"  },
    { 'o', 0, "Mixed Pool Strategy", NULL, "Key item placement strategy for the mixed pool. Requires at least one option on 'pool'\n"
                                           "Balanced will keep the original distribution of key items per pool (4 or 5 in gourds, 2 in alchemy, 11 in boss drops)\n"
                                           "Random will randomly distribute key items into any selected pool\n"
                                           "Bosses will try to place all key items into boss drops. Requires boss dropamizer on 'pool'", POOL_STRATEGY_VALUES, "General", "Key items" },
    { 'v', 0, "Sniff ingredients", NULL,   "Vanilla or random ingredient drops", VANILLA_RANDOM, "General", "Other"  },
    { 'i', 1, "Ingredienizer", NULL,       "Shuffle ('on') or randomize ('full') ingredients required for formulas", OFF_ON_FULL, "General", "Other" },
    { 'c', 1, "Callbeadamizer", NULL,      "Shuffle call bead characters ('on') or shuffle individual spells ('full')", OFF_ON_FULL, "General", "Other" },
    { 'd', 0, "Doggomizer", "Act1-3",      "Random dog per act ('on') or per room ('full')", OFF_ON_FULL, "General", "Other" },
    { 'p', 0, "Pupdunk mode", "Act0 dog",  "Everpupper everywhere! Overrides Doggomizer", OFF_ON, "General", "Other" },
    { 'm', 0, "Musicmizer", "Demo",        "Random music for some rooms", OFF_ON, "General", "Cosmetic" },
    { 'j', 0, "Sequence breaks", NULL,     "Off (default) fixes some sequence breaks: Volcano rock, final boss hatch (not out of bounds). "
                                           "'Logic' may expect the player to break sequence or skip invisible bridges to finish.", OFF_ON_LOGIC, "Accessibility", NULL },
    { 'u', 0, "Out of bounds", NULL,       "Off (default) fixes dog collision when leaving West of Crustacia. "
                                           "'Logic' may expect a player to use OoB to finish.", OFF_ON_LOGIC, "Accessibility", NULL },
    { '2', 0, "Allow cheats", NULL,        "Don't fix vanilla \\\"cheats\\\": Infinite call beads", OFF_ON, "Accessibility", NULL },
    { '5', 1, "Allow infinite ammo", NULL, "Don't fix bug that would have bazooka ammo not drain", OFF_ON, "Accessibility", NULL },
    { '6', 1, "Allow atlas glitch", NULL,  "Don't fix status effects cancelling with pixie dust", OFF_ON, "Accessibility", NULL },
    { '7', 1, "Allow wings glitch", NULL,  "Don't fix wings granting invincibility if they \\\"did not work\\\"", OFF_ON, "Accessibility", NULL },
    { 'f', 1, "Short boss rush", NULL,     "Start boss rush at Metal Magmar, cut HP in half", OFF_ON, "Accessibility", NULL },
    { 'k', 1, "Keep dog", NULL,            "Keep dog in some places to avoid softlocks", OFF_ON, "Quality of Life", NULL },
    { '9', 1, "Shorter dialogs", "Few",    "Shorten some dialogs/cutscenes. Ongoing effort.", OFF_ON, "Quality of Life", NULL },
    { 't', 0, "Turdo Mode", NULL,          "Replaces all offensive spells with \\\"turd balls\\\", weakens weapons, reduces enemies' Magic Def.", OFF_ON, "\\\"Fun\\\"", NULL },
};
enum option_indices {
    openworld_idx,
    accessible_idx,
    spoilerlog_idx,
    energy_core_idx,
    alchemizer_idx,
    bossdropamizer_idx,
    gourdomizer_idx,
    sniffamizer_idx,
    mixedpool_idx,
    sniffingredients_idx,
    ingredienizer_idx,
    callbeadamizer_idx,
    doggomizer_idx,
    pupdunk_idx, 
    /*enemizer_idx,*/
    musicmizer_idx,
    sequencebreaks_idx,
    oob_idx,
    cheats_idx,
    ammoglitch_idx,
    atlasglitch_idx,
    wingsglitch_idx,
    shortbossrush_idx, 
    keepdog_idx,
    shortdialogs_idx,
    turdomode_idx,
};
#define D(IDX) options[ IDX ].def
#define O(IDX) option_values[ IDX ]
#define C(IDX) ( O(IDX) != D(IDX) )
#define openworld O(openworld_idx)
#define keepdog O(keepdog_idx)
#define sequencebreaks O(sequencebreaks_idx)
#define oob O(oob_idx)
#define cheats O(cheats_idx)
#define accessible O(accessible_idx)
#define ammoglitch O(ammoglitch_idx)
#define atlasglitch O(atlasglitch_idx)
#define wingsglitch O(wingsglitch_idx)
#define shortdialogs O(shortdialogs_idx)
#define ingredienizer O(ingredienizer_idx)
#define alchemizer O(alchemizer_idx)
#define bossdropamizer O(bossdropamizer_idx)
#define gourdomizer O(gourdomizer_idx)
#define sniffamizer O(sniffamizer_idx)
#define mixedpool O(mixedpool_idx)
#define sniffingredients O(sniffingredients_idx)
#define callbeadamizer O(callbeadamizer_idx)
#define doggomizer O(doggomizer_idx)
#define pupdunk O(pupdunk_idx)
#define enemizer O(enemizer_idx)
#define musicmizer O(musicmizer_idx)
#define shortbossrush O(shortbossrush_idx)
#define turdomode O(turdomode_idx)
#define spoilerlog O(spoilerlog_idx)
#define energy_core O(energy_core_idx)

#define DEFAULT_SETTINGS() do {\
    difficulty=DEFAULT_difficulty;\
    for (size_t i=0; i<ARRAY_SIZE(options); i++)\
        O(i) = D(i);\
} while (false)

#define SETTINGS2STR(s)\
    do {\
        char* t = s;\
        assert(ARRAY_SIZE(s)>ARRAY_SIZE(options)+2);\
        *t++ = 'r'; *t++ = DIFFICULTY_CHAR[difficulty];\
        for (size_t i=0; i<ARRAY_SIZE(options); i++)\
            if (C(i) && options[i].key) *t++ = ( (O(i)==FULL) ? toupper(options[i].key) : tolower(options[i].key) );\
        *t++ = 0;\
    } while (false)

#ifdef __EMSCRIPTEN__
struct preset {const char* name; const char* settings; int exp; int money;};
const static struct preset presets[] = {
    {"First-Timer", "rel", 300, 300},
    {"Beginner", "rel", 200, 250},
    {"Advanced", "rlABGvCd67", 150, 200},
    {"Pro", "rhlABGIvCD567", 125, 150},
    {"Hell", "rhlABGOISvCp567f", 75, 75},
    {"Menblock", "rhlABGOISvCp567f", 1, 1},
    {"Turdo", "rxlABGoISvCDm567t", 125, 200},
    {"Full Random", "rxlABGoISvCDm67", -1, -1} //-1 => weekly-esque random value
};
#endif

#ifdef NO_UI
#define _FLAGS "[-o <dst file.sfc>|-d <dst directory>] [--dry-run] [--money <money%%>] [--exp <exp%%>] " \
               "[--available-fragments <n>] [--required-fragments <n>] "
#else
#define _FLAGS "[-b|-i] [-o <dst file.sfc>|-d <dst directory>] [--dry-run] [--money <money%%>] [--exp <exp%%>] "\
               "[--available-fragments <n>] [--required-fragments <n>] [--mystery] "
#endif
#ifdef WITH_MULTIWORLD
#define FLAGS _FLAGS "[--id <128 hex nibbles>] [--placement <placement.txt>] [--death-link] "
#else
#define FLAGS _FLAGS
#endif

#define APPNAME "Evermizer"
#define ARGS " [settings [seed]]"


// The actual program
static void print_json(const char* str)
{
    // escape \n to \\n
    while (true) {
        const char* lf = strchr(str, '\n');
        if (lf) {
            printf("%.*s\\n", (int)(lf-str), str);
            str = lf+1;
        } else {
            printf("%s", str);
            break;
        }
    }
}

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
    fprintf(stderr, "       %s --presets.json     available presets as json\n", appname);
#endif
#if defined(WIN32) || defined(_WIN32)
    fprintf(stderr, "       or simply drag & drop your ROM onto the EXE\n");
#ifndef NO_UI
    if (!batch) pause();
#endif
#endif
}

#ifndef __EMSCRIPTEN__
static void print_settings()
{
    printf("%s %s settings:\n", APPNAME, VERSION);
    printf("Difficulty:\n");
    for (uint8_t i=0; i<ARRAY_SIZE(DIFFICULTY_CHAR); i++)
        printf("  %c: %s%s\n", DIFFICULTY_CHAR[i], DIFFICULTY_NAME[i],
                               i==DEFAULT_difficulty?" (default)":"");
    printf("Options:\n");
    for (size_t i=0; i<ARRAY_SIZE(options); i++) {
        const struct option* opt = options+i;
        if (! opt->key || ! opt->state_names) continue;
        const char* defaultTextModifier = strcmp(opt->state_names[!opt->def],"Off")==0 ? "No " :
                                          strcmp(opt->state_names[!opt->def],"On")==0 ? "" :
                                          opt->state_names[!opt->def];
        const char* defaultTextSpace = (defaultTextModifier == opt->state_names[!opt->def]) ? " " : "";
        printf("  %c: %s%s%s%s%s%s\n", tolower(opt->key), defaultTextModifier, defaultTextSpace,
                                       opt->text, opt->info?" [":"", opt->info?opt->info:"", opt->info?"]":"");
        if (opt->state_names[2])
            printf("  %c: %s %s\n", toupper(opt->key), opt->state_names[2], opt->text);
        if (opt->description)
            printf("        %s\n", opt->description);
    }
    printf("\n");
}
#endif

static void print_settings_json()
{
    printf("{\"Difficulty\": [\n");
    for (uint8_t i=0; i<ARRAY_SIZE(DIFFICULTY_CHAR); i++) {
        if (i != 0) printf(",\n");
        printf("  [ \"%c\", \"%s\", %s ]", DIFFICULTY_CHAR[i], DIFFICULTY_NAME[i],
                               i==DEFAULT_difficulty?"true":"false");
    }
    printf("\n ],\n");
    printf(" \"Difficulty Description\": "
            "\"Judges item placement based on logic depth (amount of key items needed to reach the goal) "
            "and gameplay difficulty (early call beads, strong weapons or spells, hard-to-reach-checks, etc.) "
            "and will reroll until these parameters are within bounds for the selected difficulty. "
            "Also affects spell cost with Ingredienizer.\\n"
            "Easy will exclude key items at hidden gourds (behind walkthru walls and not near alchemist) "
            "from Gourdomizer and make sure Atlas is usable before act 4.\\n"
            "Normal will make sure Atlas is reachable before boss rush.\\n"
            "Random will allow any seed.\",\n");
    printf(" \"Options\": [\n");
    bool first_opt = true;
    for (size_t i=0; i<ARRAY_SIZE(options); i++) {
        const struct option* opt = options+i;
        if (! opt->key) continue;
        if (!first_opt) printf(",\n");
        first_opt = false;
        printf("  [ \"%c\", \"%s%s%s%s\", %d, \"", opt->key, opt->text,
                  opt->info?" [":"", opt->info?opt->info:"", opt->info?"]":"",
                  opt->def);
        print_json(opt->description?opt->description:"");
        printf("\", [");
        for (size_t j=0; opt->state_names && opt->state_names[j]; j++) {
            if (j != 0) printf(", ");
            printf("\"%s\"", opt->state_names[j]);
        }
        printf("], \"%s\", \"%s\" ]", opt->section, opt->subsection==NULL?"":opt->subsection);
    }
    printf("\n ],\n");
    printf(" \"Args\": [\n");
    printf("  [ \"exp\", \"Exp%%\", \"int\", \"100\", \"Character, alchemy and weapon experience modifier in percent. (1-2500)\",  \"Accessibility\", \"\", 1, 2500 ],\n");
    printf("  [ \"money\", \"Money%%\", \"int\", \"100\", \"Enemy money modifier in percent. (1-2500)\", \"Accessibility\", \"\", 1, 2500 ],\n");
    printf("  [ \"required-fragments\",  \"Required fragments\",  \"int\", \"10\","
                "\"Number of fragments required to get Energy Core in 'Fragment' mode. (1-99)\","
                "\"General\", \"Key items\", 1, 99 ],\n");
    printf("  [ \"available-fragments\", \"Available fragments\", \"int\", \"11\","
                "\"Number of fragments placed in the world in 'Fragment' mode. (1-99)\","
                "\"General\", \"Key items\", 1, 99 ]\n");
    printf(" ]\n}\n\n");
}

#ifdef __EMSCRIPTEN__
static void print_presets_json()
{
    printf("[\n");
    bool first = true;
    for (size_t i=0; i<ARRAY_SIZE(presets); i++) {
        const struct preset* preset = presets+i;
        if (! preset->name) continue;
        if (!first) printf(",\n");
        first = false;
        printf(" {\n");
        printf("  \"Name\": \"%s\",\n", preset->name);
        printf("  \"Settings\": \"%s\",\n", preset->settings);
        printf("  \"Args\": [\n");
        printf("   [ \"exp\", %i],\n", preset->exp);
        printf("   [ \"money\", %i]\n", preset->money);
        printf("  ]\n");
        printf(" }");
    }
    printf("\n]\n");
}
#endif

static void shuffle_pools(uint16_t* pool1, size_t len1, uint16_t* pool2, size_t len2, uint8_t strategy)
{
    assert(len2<65536);

    if (strategy == STRATEGY_BALANCED) {
        // iterate over shorter pool1. 50:50 chance to swap with an item from pool2 of the same type (key / non-key)
        size_t key2_count = count_real_progression_from_packed(pool2, len2);
        size_t nonkey2_count = len2-key2_count;
        assert(nonkey2_count>0);
        // cache key and non-key item indices from pool2
        size_t* key2_indices = calloc(key2_count, sizeof(size_t));
        size_t* nonkey2_indices = calloc(nonkey2_count, sizeof(size_t));
        for (size_t i=0, j=0, k=0; i<len2; i++) {
            if (is_real_progression_from_packed(pool2[i])) {
                assert(key2_count);
                key2_indices[j++] = i;
            } else {
                nonkey2_indices[k++] = i;
            }
        }
        for (size_t i=0; i<len1; i++) {
            if (rand_u8(0,1)) {
                if (is_real_progression_from_packed(pool1[i])) {
                    if (!key2_count)
                        continue;
                    // swap with key item from pool2
                    size_t n = rand_u16(0, key2_count-1);
                    SWAP(pool1[i], pool2[key2_indices[n]], uint16_t);
                } else {
                    // swap with non-key item from pool2
                    size_t n = rand_u16(0, nonkey2_count-1);
                    SWAP(pool1[i], pool2[nonkey2_indices[n]], uint16_t);
                }
            }
        }
        // cleanup
        free(key2_indices);
        free(nonkey2_indices);
    }
    else if (strategy == STRATEGY_RANDOM) {
        // iterate over shorter pool1. 50:50 chance to swap with *any* item from pool2
        for (size_t i=0; i<len1; i++) {
            if (rand_u8(0,1)) {
                size_t j = rand_u16(0, (uint16_t)len2-1);
                SWAP(pool1[i], pool2[j], uint16_t);
            }
        }
    }
    else if (strategy == STRATEGY_BOSSES) {
        // pool1 has to be bosses
        // iterate over pool2. swap all key items with non-key from pool2
        size_t nonkey1 = len1-count_real_progression_from_packed(pool1,len1);
        for (size_t i=0; i<len2; i++) {
            if (nonkey1 == 0) {
                int tmp1 = count_real_progression_from_packed(pool1, len1);
                int tmp2 = count_real_progression_from_packed(pool2, len2);
                printf("no more spots to fill (%d, %d)\n", tmp1, tmp2);
                break;
            }
            if (is_real_progression_from_packed(pool2[i])) {
                size_t n = rand_u16(0, (uint16_t)nonkey1-1);
                size_t j = 0;
                while (true) {
                    if (!is_real_progression_from_packed(pool1[j])) {
                        if (n == 0)
                            break;
                        n--;
                    }
                    j++;
                    assert(j < len1); // otherwise we miscounted somewhere
                }
                SWAP(pool1[j], pool2[i], uint16_t);
                nonkey1--;
            }
        }
        // iterate over pool1. 50:50 chance to swap non-key item
        for (size_t i=0; i<len1; i++) {
            if (!is_real_progression_from_packed(pool1[i])) {
                if (rand_u8(0,1)) {
                    size_t j = rand_u16(0, (uint16_t)len2-1);
                    assert(!is_real_progression_from_packed(pool2[j])); // pool2 should not have any more key items
                    SWAP(pool1[i], pool2[j], uint16_t);
                }
            }
        }
    } else {
        // bad strategy
        assert(false);
    }
}

int main(int argc, const char** argv)
{
#ifndef __EMSCRIPTEN__
    const char* appname = argv[0];
#else
    const char* appname = APPNAME;
#endif
    
    // verify at least one argument is given
    if (argc<2 || !argv[1] || !argv[1][0]) {
        print_usage(appname);
        exit(1);
    }

    uint8_t difficulty;
    uint8_t option_values[ARRAY_SIZE(options)] = {0};

    DEFAULT_SETTINGS();

    bool verify = false; // verify ROM and exit
    bool dry = false; // dry run: don't write ROM

    const char* ofn = NULL;
    const char* dstdir = NULL;
    bool modeforced = false;
    #if !defined NO_UI
    bool interactive; // set later
    #endif

    uint8_t money_num = 0;
    uint8_t money_den = 0;
    uint8_t exp_num = 0;
    uint8_t exp_den = 0;
    uint8_t available_fragments = 11;
    uint8_t required_fragments = 10;

    bool mystery = false;

    #ifdef WITH_MULTIWORLD
    uint8_t id_data[64];
    bool id_data_set = false;
    const size_t id_loc = 0x3d0040;
    const char* placement_file = NULL;
    bool death_link = false;
    #else
    #define placement_file false
    #define death_link false
    #endif
    
    // parse command line arguments
    while (argc>1) {
        if (strcmp(argv[1], "-b") == 0) {
            modeforced = true;
            batch = true;
    #if !defined NO_UI
            interactive = false;
    #endif
            argv++; argc--;
        } else if (strcmp(argv[1], "-i") == 0) {
            modeforced = true;
    #if defined NO_UI
            fprintf(stderr, "Requested interactive mode, but not compiled in!\n");
            print_usage(appname);
            exit(1);
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
    #ifdef __EMSCRIPTEN__
        } else if (strcmp(argv[1], "--presets.json") == 0) {
            print_presets_json();
            return 0;
    #endif
        } else if (strcmp(argv[1], "--verify") == 0) {
            argv++; argc--;
            verify=true;
        } else if (strcmp(argv[1], "--dry-run") == 0) {
            argv++; argc--;
            dry=true;
        } else if (strcmp(argv[1], "--money") == 0 && argc > 2) {
            int money = atoi(argv[2]);
            if (money>2500) money=2500; // limit to 25x
            if (money>0) percent_to_u8_fraction(money, &money_num, &money_den);
            argv+=2; argc-=2;
        } else if (strcmp(argv[1], "--exp") == 0 && argc > 2) {
            int exp = atoi(argv[2]);
            if (exp>2500) exp=2500; // limit to 25x
            if (exp>0) percent_to_u8_fraction(exp, &exp_num, &exp_den);
            argv+=2; argc-=2;
        } else if (strcmp(argv[1], "--required-fragments") == 0 && argc > 2) {
            // NOTE: fragment count will be checked but ignored for normal core
            int val = atoi(argv[2]);
            if (val<1 || val>99) {
                fprintf(stderr, "Required fragments has to be in 1..99\n");
                break;
            }
            required_fragments = (uint8_t)val;
            argv+=2; argc-=2;
        } else if (strcmp(argv[1], "--available-fragments") == 0 && argc > 2) {
            // NOTE: fragment count will be checked but ignored for normal core
            int val = atoi(argv[2]);
            if (val<1 || val>99) {
                fprintf(stderr, "Available fragments has to be in 1..99\n");
                break;
            }
            available_fragments = (uint8_t)val;
            argv+=2; argc-=2;
    #ifdef WITH_MULTIWORLD
        } else if (strcmp(argv[1], "--id") == 0 && argc > 2) {
            id_data_set = parse_id(id_data, sizeof(id_data), argv[2]);
            if (!id_data_set) {
                fprintf(stderr, "Error in id syntax\n");
                break;
            }
            argv+=2; argc-=2;
        } else if (strcmp(argv[1], "--placement") == 0 && argc > 2) {
            placement_file = argv[2];
            argv+=2; argc-=2;
        } else if (strcmp(argv[1], "--death-link") == 0) {
            argv++; argc--;
            death_link = true;
    #endif
        } else if (strcmp(argv[1], "--mystery") == 0) {
            argv++; argc--;
            mystery = true;
        } else {
            break;
        }
    }

    if (!modeforced) {
        #if !defined NO_UI
        interactive = argc<4;
        #endif
    }
    
    // verify number of command line arguments
    if (argc<2 || !argv[1] || !argv[1][0] || argc>4) {
        print_usage(appname);
        exit(1);
    }
    
    // parse settings command line argument
    if (argc>=3) {
        for (const char* s=argv[2]; *s; s++) {
            char c = *s;
            for (size_t i=0; i<ARRAY_SIZE(DIFFICULTY_CHAR); i++) {
                if (c == DIFFICULTY_CHAR[i]) { difficulty = (uint8_t)i; c = 0; }
            }
            for (size_t i=0; i<ARRAY_SIZE(options); i++) {
                if (options[i].key && c == tolower(options[i].key)) {
                    option_values[i] = options[i].def ? 0 : 1;
                    c=0; break;
                }
                if (options[i].key && c == toupper(options[i].key)) {
                    if (options[i].state_names && options[i].state_names[2]) {
                        option_values[i] = 2;
                    } else {
                        option_values[i] = options[i].def ? 0 : 1;
                    }
                    c=0; break;
                }
            }
            if (c == 'r') DEFAULT_SETTINGS();
            else if (c != 0) {
                fprintf(stderr, "Unknown setting '%c' in \"%s\"\n",
                        c, argv[2]);
                die(NULL);
            }
        }
        if (turdomode)
            ammoglitch=0;
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
    if (sz != 3145728 && sz != 3145728+512 && sz != 3670016 && sz != 3670016+512) {
        fclose(fsrc);
        die("ROM has to be 3MB or 3.5MB SFC with or without header!\n");
    }
    fseek(fsrc, 0L, SEEK_SET);
    
    const size_t rom_off = (sz == 3145728+512 || sz == 3670016+512) ? 512 : 0;
    bool grow = false; // will be set by patches if gowing the rom is required
    #define GROW_BY (4*1024*1024-sz+rom_off) // 0.5 or 1MB to a round 4MB
    assert(GROW_BY == 1024*1024 || GROW_BY == 512*1024);
    
    uint8_t* buf = (uint8_t*)malloc(sz+GROW_BY); // allow to grow by 1MB
    memset(buf+sz, 0, GROW_BY); // or 0xff?
    size_t len = fread(buf, 1, sz, fsrc);
    if (len!=sz) { free(buf); fclose(fsrc); die("Could not read input file!\n"); }
    
    // check ROM header
    const char cart_header[] = "SECRET OF EVERMORE   \x31\x02\x0c\x03\x01\x33\x00";
    const char soe2p_header[]= "SoE 2-Player FuSoYa  \x31\x02\x0c\x03\x01\x33\x00";
    const size_t cart_header_loc = 0xFFC0;
    bool is_2p = false;
    if (memcmp((char*)buf + rom_off + cart_header_loc, cart_header, sizeof(cart_header)-1) == 0)
    {
        // ok
    }
    else if (memcmp((char*)buf + rom_off + cart_header_loc, soe2p_header, sizeof(soe2p_header)-1) == 0)
    {
        // SoE 2P
        is_2p = true;
    }
    else
    {
        // unknown/unsupported rom
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
        printf("OK\n");
        free(buf);
        fclose(fsrc);
        return 0;
    }
    
    // show command line settings in batch mode
    char settings[ARRAY_SIZE(options)+3];
    SETTINGS2STR(settings);

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
            char seedbuf[18];
            printf("Seed (ENTER for random): ");
            fflush(stdout);
            if (! fgets(seedbuf, sizeof(seedbuf), stdin)) die("\nAborting...\n");
            if (isxdigit(seedbuf[0])) seed = strtoull(seedbuf, NULL, 16);
        }
        if (!exp_den) {
            char expbuf[6];
            printf("Exp%%: ");
            if (!money_den) printf("  "); // align
            fflush(stdout);
            if (! fgets(expbuf, sizeof(expbuf), stdin)) die("\nAborting...\n");
            int exp = atoi(expbuf);
            if (exp>2500) exp=2500;
            if (exp>0) percent_to_u8_fraction(exp, &exp_num, &exp_den);
        }
        if (!money_den) {
            char moneybuf[6];
            printf("Money%%: ");
            fflush(stdout);
            if (! fgets(moneybuf, sizeof(moneybuf), stdin)) die("\nAborting...\n");
            int money = atoi(moneybuf);
            if (money>2500) money=2500; // limit to 25x
            if (money>0) percent_to_u8_fraction(money, &money_num, &money_den);
        }
        while (true) {
            clrscr();
            printf(APPNAME " " VERSION "\n");
            printf("Seed: %" PRIx64 "  Exp:%4d%%  Money:%4d%%\n", seed,
                u8_fraction_to_percent(exp_num,exp_den),
                u8_fraction_to_percent(money_num,money_den));
            SETTINGS2STR(settings);
            printf("Settings: %-20s(Press R to reset)\n", settings);
            printf("\n");
            printf("Difficulty:          %-9s(%c",
                    DIFFICULTY_NAME[difficulty], toupper(DIFFICULTY_CHAR[0]));
            for (size_t i=1; i<ARRAY_SIZE(DIFFICULTY_CHAR); i++)
                printf("/%c", toupper(DIFFICULTY_CHAR[i]));
            printf(" to change)\n");
            for (size_t i=0; i<ARRAY_SIZE(options); i++) {
                const struct option* opt = options+i;
                char col1[32]; snprintf(col1, sizeof(col1), "%s:", opt->text);
                printf("%-20s %-9s%c%c%s%s%s%s\n", col1, opt->state_names[O(i)],
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
                if (c == DIFFICULTY_CHAR[i]) difficulty = (uint8_t)i;
            for (size_t i=0; i<ARRAY_SIZE(options); i++) {
                if (c == options[i].key) {
                    option_values[i]++;
                    if (!options[i].state_names[option_values[i]])
                        option_values[i]=0;
                }
            }
            if (c == 'r') DEFAULT_SETTINGS();
            if (turdomode)
                ammoglitch=0;
        }
        clrscr();
    }
    #endif
    printf(APPNAME " " VERSION "\n");
    printf("Seed: %" PRIx64 "\n", seed);
    srand64(seed);
    bool randomized = alchemizer || ingredienizer || bossdropamizer ||
                      gourdomizer || sniffamizer || doggomizer || callbeadamizer ||
                      sniffingredients || placement_file /*||enemizer*/;
    bool randomized_difficulty = alchemizer || ingredienizer || bossdropamizer ||
                      gourdomizer;
    printf("Settings: %-10s\n\n", settings);

    if (placement_file) {
        available_fragments = required_fragments;
    } else if (energy_core == ENERGY_CORE_FRAGMENTS) {
        // validate and fix up values for core fragments
        // NOTE: with rising number of required fragments, difficulty balancing becomes near impossible
        if (required_fragments > 40 && difficulty < 1) {
            free(buf);
            die("Easy only supports 40 fragments.\n");
        }
        if (required_fragments > 55 && difficulty < 2) {
            free(buf);
            die("Normal only supports 55 fragments.\n");
        } else if (available_fragments < required_fragments) {
            available_fragments = required_fragments;
            fprintf(stderr, "Warning: Available fragments too low.\n"
                    "Changing available to required\n");
        }
    }

    // define patches
    #define DEF_LOC(n, location)\
        const size_t PATCH_LOC##n = location
    #define DEF(n, location, content)\
        const size_t PATCH_LOC##n = location;\
        const char PATCH##n[] = content
    
    #include "patches.h" // hand-written c code patches
    #include "gen.h" // generated from patches/
    #include "doggo.h" // generated list of doggo changes
    
    DEF(JUKEBOX_SJUNGLE,      0x938664 - 0x800000, "\x29\x00\x03\x0f"); // CALL jukebox1
    DEF(JUKEBOX_RAPTORS_1,    0x9391fa - 0x800000, "\x29\x14\x03\x0f\x4d\x4d"); // CALL jukebox3, NOP, NOP
    DEF(JUKEBOX_RATPROS_3,    0x938878 - 0x800000, "\x29\x00\x03\x0f"); // CALL jukebox1
    DEF(JUKEBOX_DEFEND,       0x94e5b9 - 0x800000, "\x29\x00\x03\x0f"); // CALL jukebox1
    DEF(JUKEBOX_NJUNGLE,      0x939664 - 0x800000, "\x29\x00\x03\x0f"); // CALL jukebox1
    DEF(JUKEBOX_EJUNGLE,      0x93b28a - 0x800000, "\x29\x00\x03\x0f"); // CALL jukebox1
    DEF(JUKEBOX_ECRUSTACIA,   0x95bb46 - 0x800000, "\x29\x00\x03\x0f"); // CALL jukebox1
    DEF(JUKEBOX_ECRUSTACIAR,  0x95ba0b - 0x800000, "\x29\x00\x03\x0f"); // CALL jukebox1
    DEF(JUKEBOX_CRUSTACIAFP,  0x97c125 - 0x800000, "\x29\x00\x03\x0f"); // CALL jukebox1
    DEF(JUKEBOX_NOBILIAF,     0x95d72c - 0x800000, "\x29\x00\x03\x0f"); // CALL jukebox1
    DEF(JUKEBOX_NOBILIAFP,    0x97c579 - 0x800000, "\x29\x00\x03\x0f"); // CALL jukebox1
    DEF(JUKEBOX_STRONGHHE,    0x94e625 - 0x800000, "\x29\x00\x03\x0f"); // CALL jukebox1
    DEF(JUKEBOK_SWAMPPEPPER,  0x94dde6 - 0x800000, "\x29\x00\x03\x0f"); // CALL jukebox1
    DEF(JUKEBOK_SWAMPSLEEP,   0x94def3 - 0x800000, "\x29\x00\x03\x0f"); // CALL jukebox1
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
  //DEF(JUKEBOX_PALACE,       0x95d43f - 0x800000, "\x29\x70\x00\x0f"); // CALL jukebox1 // wall sounds in past-vigor cutscene glitch out

    struct formula ingredients[ALCHEMY_COUNT];
    // preset to vanilla for logic checking without ingredienizer
    {
        const uint8_t ingredient_types[] = INGREDIENT_TYPES; // includes laser
        const uint8_t ingredient_amounts[] = INGREDIENT_AMOUNTS; // includes laser
        for (size_t i=0; i<ALCHEMY_COUNT; i++) {
            ingredients[i].type1 = ingredient_types[2*alchemy_locations[i].id+0];
            ingredients[i].type2 = ingredient_types[2*alchemy_locations[i].id+1];
            ingredients[i].amount1 = ingredient_amounts[2*alchemy_locations[i].id+0];
            ingredients[i].amount2 = ingredient_amounts[2*alchemy_locations[i].id+1];
        }
    }
    // NOTE: alchemy, boss_drops and gourd_drops are 6 msb type + 10 lsb index
    // NOTE: the same init is repeated below during fill to reset everything
    uint16_t alchemy[ALCHEMY_COUNT];
    _Static_assert(ARRAY_SIZE(alchemy) == 34, "Bad alchemy count");
    const uint16_t vanilla_boss_drops[] = BOSS_DROPS;
    uint16_t gourd_drops[ARRAY_SIZE(gourd_drops_data)];
    uint16_t boss_drops[ARRAY_SIZE(vanilla_boss_drops)];
    // similar to traps and extras, we encode the actual item in the (lower 10 bits of) sniff drops
    uint16_t sniff_drops[ARRAY_SIZE(sniff_data)];
    // dog swap data
    uint8_t doggo_map[ARRAY_SIZE(doggo_vals)]; // for non-full only
    for (size_t i=0; i<ARRAY_SIZE(doggo_map); i++)
        doggo_map[i] = doggo_vals[i];
    uint8_t doggo_changes[ARRAY_SIZE(doggos)]; // preset to vanilla or pupdunk
    for (size_t i=0; i<ARRAY_SIZE(doggo_changes); i++) {
        doggo_changes[i] = (pupdunk) ? doggo_vals[0] : doggos[i].val;
    }
    // vanilla call bead spell / menu IDs, can be shuffled
    uint8_t callbead_menus[] = {0,2,4,6};
    uint16_t callbead_spells[] = {
        0x00f8, 0x00fa, 0x00fc, 0x00fe, // fire eyes
        0x0100, 0x0102, 0x0104,         // horace (without regenerate and aura)
        0x010a, 0x010c, 0x010e, 0x0110, // camellia
        0x0112, 0x0114, 0x0116          // sidney
    };

#ifdef WITH_MULTIWORLD
    if (placement_file) {
        for (size_t i=0; i<ALCHEMY_COUNT; i++)
            alchemy[i] = (CHECK_ALCHEMY<<10) + (uint16_t)i;
        for (size_t i=0; i<ARRAY_SIZE(boss_drops); i++)
            boss_drops[i] = (CHECK_BOSS<<10) + vanilla_boss_drops[i];
        for (size_t i=0; i<ARRAY_SIZE(gourd_drops); i++)
            gourd_drops[i] = (CHECK_GOURD<<10) + (uint16_t)i;
        if (sniffingredients) {
            // fill sniff spots with random ingredients
            for (size_t i=0; i<ARRAY_SIZE(sniff_drops); i++)
                sniff_drops[i] = (CHECK_SNIFF<<10) + rand_u16(0x0200, 0x0200 + ARRAY_SIZE(ingredient_names) - 1);
        } else {
            // fill sniff spots with vanilla ingredients
            for (size_t i=0; i<ARRAY_SIZE(sniff_drops); i++)
                sniff_drops[i] = (CHECK_SNIFF<<10) + (sniff_data[i].item & 0x3ff); // NOTE: this is 0x01f for iron bracer
        }

        FILE* f = fopen(placement_file, "rb");
        if (!f || fseek(f, 0, SEEK_END) != 0) {
            free(buf);
            die("Could not open placement file!\n");
        }
        long placement_size = ftell(f);
        rewind(f);
        char* placement = (char*)malloc(placement_size+1);
        if (placement && placement_size>=0 && fread(placement, 1, placement_size, f) != (size_t)placement_size) {
            free(buf);
            free(placement);
            fclose(f);
            die("Error reading placement data!\n");
        }
        placement[placement_size] = 0;
        fclose(f);
        char* placement_line = placement;
        do {
            char* lf = strchr(placement_line, '\n');
            if (lf && lf>placement_line && *(lf-1) == '\r')
                *(lf-1) = '\0';
            if (lf) *lf = '\0';
            // format in placement_line:
            // <location_type>,<location_index>:<item_type>,<item_index>[,<optional>]
            // for remote items, type is 0, index is player and optional is MW location id
            char* next = placement_line;
            long loc_type = (*next) ? strtol(placement_line, &next, 10) : -1;
            long loc_index = (next && *next == ',' && loc_type > 0) ? strtol(next+1, &next, 10) : -1;
            long item_type = (next && *next == ':' && loc_index >= 0) ? strtol(next+1, &next, 10) : -1;
            long item_index = (next && *next == ',' && item_type > 0) ? strtol(next+1, &next, 10) : -1;
            if (loc_type >= 0) { // valid line
                //printf("place: %s\n", placement_line);
                if (loc_type < CHECK_ALCHEMY || loc_type > CHECK_SNIFF || item_type < 0 || item_type > CHECK_TRAP
                        || loc_index < 0 || (item_type != 0 && (item_index < 0 || item_index > 0xffff)))
                {
                    char msg[64];
                    snprintf(msg, sizeof(msg), "Invalid placement data: %s", placement_line);
                    free(buf);
                    free(placement);
                    die(msg);
                }
                if (!is_valid_placement_item((enum check_tree_item_type)item_type, (uint16_t)item_index))
                {
                    free(buf);
                    free(placement);
                    die("Unknown item in placement data!");
                }
                if (loc_type == CHECK_ALCHEMY && (size_t)loc_index < ARRAY_SIZE(alchemy)) {
                    if (item_type == 0)
                        alchemy[loc_index] = 0x01ff; // Remote // TODO: special value
                    else
                        alchemy[loc_index] = (uint16_t)(item_type << 10) + (item_index & 0x3ff);
                }
                else if (loc_type == CHECK_BOSS && (size_t)loc_index < ARRAY_SIZE(boss_drops)) {
                    if (item_type == 0)
                        boss_drops[loc_index] = 0x01ff; // Remote // TODO: special value
                    else
                        boss_drops[loc_index] = (uint16_t)(item_type << 10) + (item_index & 0x3ff);
                }
                else if (loc_type == CHECK_GOURD && (size_t)loc_index < ARRAY_SIZE(gourd_drops)) {
                    if (item_type == 0)
                        gourd_drops[loc_index] = 0x01ff; // Remote // TODO: special value
                    else
                        gourd_drops[loc_index] = (uint16_t)(item_type << 10) + (item_index & 0x3ff);
                }
                else if (loc_type == CHECK_SNIFF && (size_t)loc_index < ARRAY_SIZE(sniff_drops)) {
                    if (item_type == 0)
                        sniff_drops[loc_index] = 0x01ff; // Remote // TODO: special value
                    else
                        sniff_drops[loc_index] = (uint16_t)(item_type << 10) + (item_index & 0x3ff);
                }
                else {
                    free(buf);
                    free(placement);
                    die("Index out of range for placement data\n");
                }
            }
            if (!lf) break; // assume \0 is end of file
            placement_line = lf+1;
        } while (true);
        free(placement);
    }
#endif

    int treedepth=0;
    int cyberlogicscore=0;
    int cybergameplayscore=0;

    int rollcount=0;
    if (randomized)
        printf("Rolling");
    fflush(stdout);
    int rolllimit = 50000;
    if (difficulty == 2) rolllimit *= 2;
    if (energy_core == ENERGY_CORE_FRAGMENTS && required_fragments>=30) rolllimit *= 2;
    do {
        // general logic checking
        #define REROLL() continue;

        // FIXME: limit generation instead of rerolling too often
        if (rollcount>rolllimit-2) {
            free(buf);
            fclose(fsrc);
            char msg[256];
            snprintf(msg, sizeof(msg), "\nCould not satisfy logic in %dk tries. Giving up.\n", rolllimit/1000);
            die(msg);
        }
        if (rollcount>0) printf(".");
        if ((rollcount+strlen("Rolling"))%79 == 0) printf("\n"); else fflush(stdout); // 79 chars per line
        rollcount++;

        if (!placement_file) {
            // reset checks
            for (size_t i=0; i<ALCHEMY_COUNT; i++)
                alchemy[i] = (CHECK_ALCHEMY<<10) + (uint16_t)i;
            for (size_t i=0; i<ARRAY_SIZE(boss_drops); i++)
                boss_drops[i] = (CHECK_BOSS<<10) + vanilla_boss_drops[i];
            for (size_t i=0; i<ARRAY_SIZE(gourd_drops); i++)
                gourd_drops[i] = (CHECK_GOURD<<10) + (uint16_t)i;
            if (sniffingredients) {
                // fill sniff spots with random ingredients
                for (size_t i=0; i<ARRAY_SIZE(sniff_drops); i++)
                    sniff_drops[i] = (CHECK_SNIFF<<10) + rand_u16(0x0200, 0x0200 + ARRAY_SIZE(ingredient_names) - 1);
            } else {
                // fill sniff spots with vanilla ingredients
                for (size_t i=0; i<ARRAY_SIZE(sniff_drops); i++)
                    sniff_drops[i] = (CHECK_SNIFF<<10) + (sniff_data[i].item & 0x3ff); // NOTE: this is 0x01f for iron bracer
            }
            // assert that last gourd is energy core. required for vanilla and fragments
            assert(get_drop_from_packed(gourd_drops[ARRAY_SIZE(gourd_drops)-1])->provides[0].progress == P_ENERGY_CORE);
            // add energy core fragments; rule update is in milestone loop
            if (energy_core == ENERGY_CORE_FRAGMENTS) {
                // replace energy core with fragment
                gourd_drops[ARRAY_SIZE(gourd_drops)-1] = (CHECK_EXTRA<<10) + 0;
                // place all other fragments by replacing ingredients
                // skipping mud peppers, gun powder, grease, dry ice or meteorite to not impact logic
                int n = available_fragments - 1;
                while (n>0) {
                    uint16_t i = rand_u16(0, sizeof(gourd_drops)-1);
                    if (gourd_drops[i] != (CHECK_GOURD<<10) + (uint16_t)i)
                        continue; // already replaced
                    const struct gourd_drop_item* item = gourd_drops_data+i;
                    bool ok = false;
                    for (size_t j=0; j<ARRAY_SIZE(ingredient_names); j++) {
                        if (j == MUD_PEPPER || j == GUNPOWDER || j == GREASE || j == DRY_ICE || j == METEORITE)
                            continue;
                        if (strcmp(item->name, ingredient_names[j]) == 0) {
                            ok = true;
                            break;
                        }
                        if (strcmp(item->name+2, ingredient_names[j]) == 0) {
                            ok = true;
                            break;
                        }
                    }
                    if (ok) {
                        gourd_drops[i] = (CHECK_EXTRA<<10) + 0;
                        n--;
                    }
                }
            }

            size_t gourd_shuffle_count = ARRAY_SIZE(gourd_drops) - ((energy_core == ENERGY_CORE_VANILLA) ? 1 : 0);
            if (alchemizer)
                shuffle_u16(alchemy, ARRAY_SIZE(alchemy));
            if (gourdomizer)
                shuffle_u16(gourd_drops, gourd_shuffle_count);
            if (bossdropamizer)
                shuffle_u16(boss_drops, ARRAY_SIZE(boss_drops));
            if (sniffamizer && !sniffingredients) // no need to shuffle if ingredients are random
                shuffle_u16(sniff_drops, ARRAY_SIZE(sniff_drops));
            // mix pools
            if (alchemizer == POOL && gourdomizer == POOL) {
                shuffle_pools(alchemy, ARRAY_SIZE(alchemy), gourd_drops, gourd_shuffle_count,
                              (mixedpool==STRATEGY_BOSSES) ? STRATEGY_RANDOM : mixedpool);
            }
            if (alchemizer == POOL && bossdropamizer == POOL) {
                shuffle_pools(boss_drops, ARRAY_SIZE(boss_drops), alchemy, ARRAY_SIZE(alchemy), mixedpool);
            }
            if (gourdomizer == POOL && bossdropamizer == POOL) {
                shuffle_pools(boss_drops, ARRAY_SIZE(boss_drops), gourd_drops, gourd_shuffle_count, mixedpool);
            }
            if (sniffamizer == POOL && alchemizer == POOL) {
                shuffle_pools(alchemy, ARRAY_SIZE(alchemy), sniff_drops, ARRAY_SIZE(sniff_drops),
                              (mixedpool==STRATEGY_BOSSES) ? STRATEGY_RANDOM : mixedpool);
            }
            if (sniffamizer == POOL && gourdomizer == POOL) {
                shuffle_pools(gourd_drops, gourd_shuffle_count, sniff_drops, ARRAY_SIZE(sniff_drops),
                              (mixedpool==STRATEGY_BOSSES) ? STRATEGY_RANDOM : mixedpool);
            }
            if (sniffamizer == POOL && bossdropamizer == POOL) {
                shuffle_pools(boss_drops, ARRAY_SIZE(boss_drops), sniff_drops, ARRAY_SIZE(sniff_drops), mixedpool);
            }
            // until we fix the non-working sniff spots, move garbage into unreachable spots
            if (sniffamizer == POOL) {
                bool ok = true;
                for (size_t dst_idx=0; dst_idx<ARRAY_SIZE(sniff_drops); dst_idx++) {
                    if ((sniff_data[dst_idx].missable || sniff_data[dst_idx].excluded) &&
                            (sniff_drops[dst_idx] >> 10) != CHECK_SNIFF) {
                        ok = false;
                        for (size_t src_idx=0; src_idx<ARRAY_SIZE(sniff_drops); src_idx++) {
                            if ((sniff_drops[src_idx] >> 10) == CHECK_SNIFF) {
                                SWAP(sniff_drops[src_idx], sniff_drops[dst_idx], uint16_t);
                                ok = true;
                                break;
                            }
                        }
                    }
                }
                if (!ok)
                    REROLL();
            }
        }

        if (!placement_file && alchemizer) {
            if (difficulty==0 && !ingredienizer && alchemizer==ON) {
                // make sure that one of acid rain, flash or speed
                // is obtainable before thraxx on easy with non-pooled
                // alchemizer with ingredienizer=off
                uint8_t at = (rand64()%2) ? HARD_BALL_IDX : FLASH_IDX;
                uint8_t spell = rand_u8(0, 2);
                spell = (spell==0) ? FLASH_IDX : (spell==1) ? ACID_RAIN_IDX : SPEED_IDX;
                for (size_t i=0; i<ALCHEMY_COUNT; i++) {
                    if (alchemy[i] == (CHECK_ALCHEMY<<10) + spell) { // swap alchemy locations
                        alchemy[i] = alchemy[at];
                        alchemy[at] = (CHECK_ALCHEMY<<10) + spell;
                        break;
                    }
                }
            }
            // TODO: place one castable spell anywhere before thraxx (ingredienizer needs to run first)
        }
        if (ingredienizer) {
            uint8_t spell_cost_mod = (difficulty == 3) ? rand_u8(0,2) : difficulty;
            const uint8_t min_single_cost = 1;
            const uint8_t max_single_cost = 3;
            const uint8_t max_spell_cost = MIN(4+spell_cost_mod, 2*max_single_cost);
            int est_total_cost = 92-6 + spell_cost_mod*6; // 92/34 for vanilla
            uint8_t cheap_spell_location = (difficulty==0) ? ((rand64()%2) ? HARD_BALL_IDX : FLASH_IDX) : 0xff;
            
            int cur_total_cost = 0;
            if (ingredienizer==FULL) {
                for (uint8_t i=0; i<ALCHEMY_COUNT; i++) {
                    uint8_t type1;
                    uint8_t type2;
                    if (i == cheap_spell_location) {
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
        if (!placement_file && gourdomizer) {
            // make sure amulet of annihilation is available in ivor tower
            {
                uint8_t amuletNo = rand_u8(0,3); // this can't be sniff spot unless pooled
                uint8_t ivorGourdNo = rand_u8(0,23);
                size_t ivorGourdIdx=(size_t)-1;
                uint16_t *amuletSrc = NULL;
                for (size_t i=0; i<ARRAY_SIZE(gourd_drops); i++) {
                    if (stris(gourd_data[i].name, "Ivor Houses")) {
                        if (ivorGourdNo == 0) {
                            ivorGourdIdx = i;
                        }
                        ivorGourdNo--;
                    }
                    if (stris(get_drop_name_from_packed(gourd_drops[i]), "Amulet of Annihilation")) {
                        if (amuletNo == 0) {
                            amuletSrc = gourd_drops+i;
                        }
                        amuletNo--;
                    }
                    if (amuletSrc && (ivorGourdIdx != (size_t)-1)) break;
                }
                if (!amuletSrc) for (size_t i=0; i<ARRAY_SIZE(boss_drops); i++) {
                    if (stris(get_drop_name_from_packed(boss_drops[i]), "Amulet of Annihilation")) {
                        if (amuletNo == 0) {
                            amuletSrc = boss_drops+i;
                            break;
                        }
                        amuletNo--;
                    }
                }
                if (!amuletSrc) for (size_t i=0; i<ARRAY_SIZE(alchemy); i++) {
                    if (stris(get_drop_name_from_packed(alchemy[i]), "Amulet of Annihilation")) {
                        if (amuletNo == 0) {
                            amuletSrc = alchemy+i;
                            break;
                        }
                        amuletNo--;
                    }
                }
                if (!amuletSrc) for (size_t i=0; i<ARRAY_SIZE(sniff_drops); i++) {
                    if (stris(get_drop_name_from_packed(sniff_drops[i]), "Amulet of Annihilation")) {
                        if (amuletNo == 0) {
                            amuletSrc = sniff_drops+i;
                            break;
                        }
                        amuletNo--;
                    }
                }
                if (mixedpool == STRATEGY_BALANCED && is_real_progression_from_packed(gourd_drops[ivorGourdIdx]))
                    REROLL();
                assert(amuletSrc);
                assert(ivorGourdIdx < ARRAY_SIZE(gourd_drops));
                // cppcheck-suppress[nullPointerRedundantCheck,unmatchedSuppression]
                SWAP(gourd_drops[ivorGourdIdx], *amuletSrc, uint16_t);
            }
            // make sure wings are available in halls NE room
            {
                uint8_t wingsNo = rand_u8(0,4);
                uint8_t hallsNEGourdNo = rand_u8(0,3);
                size_t hallsNEGourdIdx=(size_t)-1;
                uint16_t *wingsSrc = NULL;
                for (size_t i=0; i<ARRAY_SIZE(gourd_drops); i++) {
                    if (stris(gourd_data[i].name, "Halls NE")) {
                        if (hallsNEGourdNo == 0) {
                            hallsNEGourdIdx = i;
                        }
                        hallsNEGourdNo--;
                    }
                    if (stris(get_drop_name_from_packed(gourd_drops[i]), "Wings")) {
                        if (wingsNo == 0) {
                            wingsSrc = gourd_drops+i;
                        }
                        wingsNo--;
                    }
                    if (wingsSrc && hallsNEGourdIdx != (size_t)-1) break;
                }
                if (!wingsSrc) for (size_t i=0; i<ARRAY_SIZE(boss_drops); i++) {
                    if (stris(get_drop_name_from_packed(boss_drops[i]), "Wings")) {
                        if (wingsNo == 0) {
                            wingsSrc = boss_drops+i;
                            break;
                        }
                        wingsNo--;
                    }
                }
                if (!wingsSrc) for (size_t i=0; i<ARRAY_SIZE(alchemy); i++) {
                    if (stris(get_drop_name_from_packed(alchemy[i]), "Wings")) {
                        if (wingsNo == 0) {
                            wingsSrc = alchemy+i;
                            break;
                        }
                        wingsNo--;
                    }
                }
                if (!wingsSrc) for (size_t i=0; i<ARRAY_SIZE(sniff_drops); i++) {
                    if (stris(get_drop_name_from_packed(sniff_drops[i]), "Wings")) {
                        if (wingsNo == 0) {
                            wingsSrc = sniff_drops+i;
                            break;
                        }
                        wingsNo--;
                    }
                }
                if (mixedpool == STRATEGY_BALANCED && is_real_progression_from_packed(gourd_drops[hallsNEGourdIdx]))
                    REROLL();
                assert(wingsSrc);
                assert(hallsNEGourdIdx < ARRAY_SIZE(gourd_drops));
                // cppcheck-suppress[nullPointerRedundantCheck,unmatchedSuppression]
                SWAP(gourd_drops[hallsNEGourdIdx], *wingsSrc, uint16_t);
            }
        }
        if (pupdunk) {
            /* set up above */
        } else if (doggomizer && doggomizer!=FULL) {
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
        if (callbeadamizer) {
            shuffle_u8(callbead_menus, ARRAY_SIZE(callbead_menus));
            if (callbeadamizer==FULL) {
                shuffle_u16(callbead_spells, ARRAY_SIZE(callbead_spells));
            }
        }

        const struct formula* levitate_formula = &ingredients[LEVITATE_IDX];
        const struct formula* revealer_formula = &ingredients[REVEALER_IDX];
        const struct formula* atlas_formula = &ingredients[ATLAS_IDX];
        {
            if (levitate_formula->type1 == METEORITE ||
                levitate_formula->type2 == METEORITE)
                    REROLL(); // reroll, unbeatable or would give away a hint
            if ((levitate_formula->type1 == DRY_ICE && levitate_formula->amount1>1) ||
                (levitate_formula->type2 == DRY_ICE && levitate_formula->amount2>1))
                    REROLL(); // reroll, unbeatable or would give away a hint
            if ((levitate_formula->type1 == DRY_ICE && levitate_formula->amount1==1) ||
                (levitate_formula->type2 == DRY_ICE && levitate_formula->amount2==1))
            {
                // no other formula may use dry ice
                if (placement_file) REROLL(); // would need to know the whole logic
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
            // two checks below should be changed to only require <=act3 if the drop is before act4
            if (!can_buy_in_act3(levitate_formula)) REROLL(); // TODO: put ingredients in logic instead
            if (!can_buy_in_act3(revealer_formula)) REROLL();
            // make sure atlas can be cast on easy in act1-3
            if (difficulty == 0) { // easy
                if (placement_file || gourdomizer) {
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
            // NOTE: this should be guaranteed by generation; at the moment only if ingredienzier is off
            // TODO: this does not work for POOL
            if (!placement_file && alchemizer!=POOL && (ingredienizer || alchemizer) && difficulty == 0) {
                uint8_t castable = 2;
                for (size_t i=0; i<ALCHEMY_COUNT; i++)
                    if ((alchemy[FLASH_IDX]==(CHECK_ALCHEMY<<10)+i || alchemy[HARD_BALL_IDX]==(CHECK_ALCHEMY<<10)+i) &&
                            !can_buy_pre_thraxx(&ingredients[i]))
                        castable--;
                if (castable<1) {
                    if (!ingredienizer) {
                        fprintf(stderr, "PRE-THRAXX INGREDIENTS BUG: "
                                "Please report this with seed and settings.\n");
                    }
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
            // make sure we have at least 98 casts of good offensive and 33 healing spells before boss rush
            // NOTE: this should be put in logic in a way that makes you find usable alchemy before act3 prison
            if (ingredienizer) {
                unsigned offensive = 0;
                unsigned healing = 0;
                for (size_t i=0; i<ALCHEMY_COUNT; i++) {
                    if (alchemy_is_good(i) && can_buy_ingredients(ingredients+i)) {
                        offensive += 99/MAX(ingredients[i].amount1, ingredients[i].amount2);
                    }
                    else if (alchemy_is_healing(i) && can_buy_ingredients(ingredients+i)) {
                        healing += 99/MAX(ingredients[i].amount1, ingredients[i].amount2);
                    }
                }
                if (offensive<98 || healing<33) REROLL();
            }
        }
        #undef REROLL
        if (placement_file) break; // can't reroll on pre-defined logic
        // tree-based milestone logic checking
        bool reroll = false;
        #define REROLL() { \
            reroll = true; \
            break; \
        }
        for (int milestone=0; milestone<2; milestone++)
        {
            // init state
            enum progression goal = milestone==0 ? P_ROCKET : P_FINAL_BOSS;
            bool allow_sequencebreaks = sequencebreaks == LOGIC;
            bool allow_saturnskip = sequencebreaks == LOGIC;
            bool allow_oob = oob == LOGIC;
            check_tree_item checks[ARRAY_SIZE(blank_check_tree)];
            memcpy(checks, blank_check_tree, sizeof(blank_check_tree));
            // update energy core fragment rule
            for (size_t i=0; i<ARRAY_SIZE(checks); i++) {
                if (checks[i].type == CHECK_RULE &&
                        checks[i].requires[0].progress == P_CORE_FRAGMENT)
                {
                    checks[i].requires[0].pieces = required_fragments;
                    checks[i].provides[0].pieces = 1;
                    checks[i].provides[0].progress = P_ENERGY_CORE;
                    break;
                }
            }
            int progress[P_END] = {0};
            int nextprogress[P_END] = {0};
            if (allow_sequencebreaks) nextprogress[P_ALLOW_SEQUENCE_BREAKS]++;
            if (allow_oob) nextprogress[P_ALLOW_OOB]++;
            bool complete=false;
            treedepth=0;
            cyberlogicscore = 0;
            cybergameplayscore = 0;
            while (!complete) {
                int ammopenalty=0;
                int wingspenalty=0;
                if (progress[goal]<1) treedepth++;
                memcpy(progress, nextprogress, sizeof(progress));
                if (treedepth == 3) {
                    if (progress[P_ARMOR]==0)
                        cybergameplayscore += 5;
                    if (progress[P_ACT2_WEAPON]==0 && progress[P_ACT3_WEAPON]==0 && progress[P_ACT4_WEAPON]==0)
                        cybergameplayscore += 5;
                    if (progress[P_JAGUAR_RING]==0)
                        cybergameplayscore += 15;
                }
                complete = true;
                for (size_t i=0; i<ARRAY_SIZE(checks); i++) {
                    if (checks[i].reached) continue;
                    if (check_requires(checks+i, goal)) continue; // don't iterate past goal
                    // TODO: don't update scores if goal was already reached to avoid bogus scores
                    //       this may require rebalancing
                    if (check_reached(checks+i, progress)) {
                        uint16_t drop_id = checks[i].type==CHECK_ALCHEMY ? alchemy[checks[i].index] :
                                           checks[i].type==CHECK_BOSS ? boss_drops[checks[i].index] :
                                           checks[i].type==CHECK_GOURD ? gourd_drops[checks[i].index] :
                                           checks[i].type==CHECK_SNIFF ? sniff_drops[checks[i].index] : 0;
                        const drop_tree_item* drop = get_drop_from_packed(drop_id);
                        check_progress(checks+i, nextprogress);
                        if (! checks[i].missable)
                            drop_progress(drop, nextprogress);
                        if (drop && checks[i].difficulty<0) {
                            cybergameplayscore -= 2*drop_provides(drop, P_CALLBEAD);
                            if (wingsglitch && drop_provides(drop, P_WINGS)) {
                                cybergameplayscore += wingspenalty-2;
                                wingspenalty = 2;
                            }
                            if (drop_provides(drop, P_ACT1_WEAPON))
                                cybergameplayscore -= 1;
                            if (drop_provides(drop, P_ACT2_WEAPON))
                                cybergameplayscore -= 4;
                            if (drop_provides(drop, P_ACT3_WEAPON))
                                cybergameplayscore -= 6;
                            if (drop_provides(drop, P_ACT4_WEAPON))
                                cybergameplayscore -= 10;
                            if (drop_provides(drop, P_ARMOR))
                                cybergameplayscore -= 2;
                            if (ammoglitch && drop_provides(drop, P_GLITCHED_AMMO) && ammopenalty < 20) {
                                cybergameplayscore += ammopenalty-20;
                                ammopenalty = 20;
                            }
                            else if (ammoglitch && drop_provides(drop, P_AMMO) && ammopenalty < 1) {
                                cybergameplayscore += ammopenalty-1;
                                ammopenalty = 1;
                            }
                            if (!ammoglitch && drop_provides(drop, P_AMMO))
                                cybergameplayscore -= 1;
                        }
                        else if (drop && check_requires(checks+i, P_ROCKET)) {
                            if (ammoglitch && drop_provides(drop, P_GLITCHED_AMMO)) {
                                cybergameplayscore += ammopenalty-15;
                                ammopenalty = 15;
                            } else if (!ammoglitch && drop_provides(drop, P_GLITCHED_AMMO))
                                cybergameplayscore -= 1;
                        }
                        if (drop && drop->provides[0].progress > P_NONE && drop->provides[0].progress < P_ARMOR) {
                            if (checks[i].difficulty>0) cyberlogicscore++; // some checks increase difficulty
                            if (checks[i].difficulty>1 && difficulty==0) cyberlogicscore+=99; // exclude hidden
                        }
                        #ifdef DEBUG_CHECK_TREE
                        printf("Reached %s\n", check2str(checks+i));
                        if (drop) printf("Got %s%s\n", checks[i].missable?"missable ":"", drop2str(drop));
                        #endif
                        complete=false;
                    }
                }
            }
            if (allow_saturnskip && goal==P_FINAL_BOSS) { /* goal optional */ }
            else if (progress[goal]<1) REROLL();
            // make sure atlas is reachable if it should be
            if (difficulty==milestone && progress[P_ATLAS]<1) REROLL();
            if (accessible && milestone==1) {
                bool not_accessible = false;
                for (size_t i=P_NONE+1; i<P_END-1; i++) {
                    if (i == P_DE || i == P_GAUGE || i == P_WHEEL || i == P_ORACLE_BONE ||
                        i == P_CORE_FRAGMENT || i == P_ALLOW_OOB || i == P_ALLOW_SEQUENCE_BREAKS) continue;
                    if (progress[i]==0) {
                        not_accessible = true;
                        break;
                    }
                }
                if (not_accessible) REROLL();
            }
        }
        #undef REROLL
        if (reroll) continue;
        if (randomized_difficulty) {
            int logicscore = (treedepth-6)*3 + cyberlogicscore;
            if (difficulty==2 && logicscore<10) continue;
            if (difficulty==0 && logicscore>10) continue;
            if (difficulty==1 && (logicscore>15 || logicscore<5)) continue; // TODO: review seeds
        }
        int spellmodifier=0;
        for (uint8_t i=0; i<ALCHEMY_COUNT; i++) {
            if (alchemy_is_good(i) && alchemy_is_cheap(&ingredients[i]))
                spellmodifier -= 2;
        }
        if (can_buy_in_act3(&ingredients[CALL_UP_IDX])) {
            cybergameplayscore -= 5;
        }
        if (spellmodifier == 0) cybergameplayscore += 10;
        else cybergameplayscore += spellmodifier;
        if (randomized_difficulty) {
            // FIXME: vanilla gameplay score gives -25 for some reason
            int gameplayscore = cybergameplayscore;
            if (difficulty>1 && difficulty!=3 && gameplayscore<-9) continue;
            if (difficulty<1 && gameplayscore>-2) continue;
            if (difficulty==1 && (gameplayscore<-15 || gameplayscore>1)) continue; // TODO: review seeds
        }
        break;
    } while (true);
    if (randomized) printf("\n");

    // apply patches
    #define APPLY(n) APPLY_PATCH(buf, PATCH##n, rom_off + PATCH_LOC##n)
    
    if (!sequencebreaks && !openworld) {
        free(buf);
        fclose(fsrc);
        die("Cannot fix glitches without applying open world patch-set!\n");
    }

    if (bossdropamizer && !openworld) {
        free(buf);
        fclose(fsrc);
        die("Cannot randomize boss drops without open world patch-set (yet)!\n");
    }

    if (openworld) {
        printf("Applying open world patch-set...\n");
        // start with windwalker unlocked
        APPLY_OW_BASE();
        // allow access to volcano and swamp with windwalker unlocked
        APPLY_ACT1_OW();
        APPLY(6);
        APPLY(7);  APPLY(8);  APPLY(9);  APPLY(10); APPLY(11); APPLY(12);
        APPLY(13); APPLY(14); APPLY(15); APPLY(16); APPLY(17); APPLY(18);
        APPLY(19); APPLY(20); APPLY(21); APPLY(22); APPLY(23); APPLY(24);
        APPLY(25); APPLY(26); APPLY(27); APPLY(28); APPLY(29); APPLY(30);
        APPLY(31); /* -32- */ APPLY(33); APPLY(34); APPLY(35); APPLY(36);
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
        APPLY(72); /*73,74: see below*/  APPLY_IMMORTALIZE();
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
        APPLY_ACT3_OW();
        APPLY(MUD_PEPPER_LIMIT);
        APPLY(TEMP); APPLY(TEMP2); // TODO: resolve underlying issue
        // v026: levitate last rock entering magmar fight to avoid softlock
        APPLY(VOLCANO);
        // v036: fix tinker setting a (now useless) bit that breaks other firepits
        APPLY(TINKER_FIX);
    }
    if (keepdog) {
        printf("Applying patches to keep the dog...\n");
        APPLY(KEEP_DOG); APPLY(KEEP_DOG2); APPLY(KEEP_DOG3);
    }
    
    // General bug fixes
    printf("Fixing vanilla softlocks...\n");
    // v009:
    APPLY(73);
    // v015:
    APPLY(141); APPLY(142);
    // v017:
    APPLY(146);
    // v025:
    APPLY(HALLS_BUGS); // (actually not a softlock but a bug)
    // v026: fix colosseum entrance with dog available
    APPLY(COLOSSEUM7);APPLY(COLOSSEUM8);
    APPLY(COLOSSEUM9);APPLY(COLOSSEUM10);
    // v029: fix softlock when leaving Tiny's lair after opening the gate
    APPLY(TINY_FIX);
    // v032: complete east prison softlock fix
    APPLY(PRISON_FIX); APPLY(PRISON_FIX2);
    // v032: fix dog getting stuck on lillypad (only required with doggomizer)
    APPLY(PALACE_GROUNDS_DOG); APPLY(PALACE_GROUNDS_DOG2);
    // v047: wrong map ref makes looting both spots impossible
    APPLY_QUICK_SAND_FIX_SNIFF();
    // v047: duplicate triggers make another impossible to loot
    APPLY_NORTH_OF_MARKET_FIX_SNIFF();

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
    // v031: make lance and stop unmissable
    APPLY(ALCHEMY_ACCESSIBLE);
    APPLY(ALCHEMY_ACCESSIBLE2);
    // v041: make sons of sth unmissable
    APPLY_SONS_OF_STH_2();

    // FIXME: test this
    if (placement_file || gourdomizer || bossdropamizer || energy_core == ENERGY_CORE_FRAGMENTS) {
        // v015: Disable collapsing bridges in pyramids
        // v041: break wall in pyramid upper floor if we have dog already
        APPLY_PYRAMID_FIXES();
    }
    if (!placement_file && bossdropamizer && !gourdomizer && progressive_armor) {
        // v030: update vanilla armor gourds to have progressive drops
        // NOTE: this requires part of gourdomizer to be applied, see below
        APPLY(PROGRESSIVE_ARMOR2); APPLY(PROGRESSIVE_ARMOR3);
        APPLY(PROGRESSIVE_ARMOR4); APPLY(PROGRESSIVE_ARMOR5);
        APPLY(PROGRESSIVE_ARMOR6); APPLY(PROGRESSIVE_ARMOR7);
        APPLY(PROGRESSIVE_ARMOR8); APPLY(PROGRESSIVE_ARMOR9);
    }
    if (!placement_file && gourdomizer && !bossdropamizer && progressive_armor) {
        // v030: update vanilla boss drops to have progressive drops
        APPLY(PROGRESSIVE_ARMOR);
    }

    // General features
    printf("Improving quality of life...\n");
    // v014:
    APPLY(106);
    // v027:
    APPLY(ALCHEMY_SELECTION_FIXES);
    // v031: fix one up dialog
    APPLY(ONEUP_FIX);
    // v032: fix bazooka weirdness
    APPLY(BAZOOKA_TWO_U);  APPLY(BAZOOKA_TWO_U2);
    APPLY(BAZOOKA_TWO_U3); APPLY(BAZOOKA_TWO_U4);
    // v033: allow to buy up to 99 atlas medallions
    APPLY(ATLAS_MEDALLIONS); APPLY(ATLAS_MEDALLIONS2);
    // v033: fix levelup mechanic (multiple levels off one enemy)
    APPLY(LEVEL_U); APPLY(LEVEL_U2); APPLY(LEVEL_U3);
    // v033: fix save+restore of wrong stats
    APPLY(STASAV_U); APPLY(STASAV_U2);
    
    if (!sequencebreaks) {
        printf("Applying desolarify patch-set...\n");
        APPLY(48);
        APPLY(49);
        printf("Applying desaturnate patch...\n");
        APPLY(68);
        printf("Disabling double gauge...\n");
        APPLY(145);
    }
    
    if (!cheats) { // cheats put in by the original devs, not glitches
        printf("Removing infinite call bead glitch...\n");
        APPLY(67);
    }
    
    if (!ammoglitch) {
        printf("Fixing infinite bazooka ammo...\n");
        APPLY(INFAMMO);
    }
    
    if (!atlasglitch) {
        printf("Fixing atlas glitch...\n");
        APPLY(STADIE_U);
    }
    
    if (!wingsglitch) {
        printf("Fixing wings glitch...\n");
        APPLY(WINGS_FIX_U);
    }
    
    if (!oob) {
        printf("Fixing OOB...\n");
        grow = true;
        APPLY_OOB_FIX();
    }
    else if (keepdog) {
        printf("Applying more patches to keep the dog...\n");
        grow = true;
        APPLY_OOB_KEEP_DOG(); // colliding with oob_fix
    }

    if ((money_num != money_den) || (exp_num != exp_den)) {
        printf("Patching enemy data...\n");
        for (uint32_t p=CHARACTER_DATA_START; p<CHARACTER_DATA_END; p+=CHARACTER_DATA_SIZE) {
            uint32_t money = read16(buf, rom_off+p+CHARACTER_DATA_MONEY_OFF);
            uint32_t exp   = read32(buf, rom_off+p+CHARACTER_DATA_EXP_OFF);
            if (money_num != money_den) money = (money * money_num + money_den/2) / money_den;
            if (exp_num   != exp_den)   exp   = (exp   * exp_num   + exp_den/2)   / exp_den;
            if (money>0xffff) money=0xffff;
            write16(buf, rom_off+p+CHARACTER_DATA_MONEY_OFF, (uint16_t)money);
            write32(buf, rom_off+p+CHARACTER_DATA_EXP_OFF,   exp);
        }
    }
    if (exp_num != exp_den) {
        printf("Patching weapon leveling...\n");
        grow = true;
        APPLY(WEAPON_EXP); APPLY(WEAPON_EXP2); APPLY(WEAPON_EXP3);
        // filling in magic numbers:
        uint16_t normal = (4 * exp_num + exp_den/2) / exp_den;
        uint16_t slow   = (2 * exp_num + exp_den/2) / exp_den;
        if (normal>255) normal=255;
        if (normal<1) normal=1;
        if (slow>255) slow=255;
        if (slow<1)   slow=1;
        write8(buf, rom_off+0x310000 +  6, (uint8_t)normal);
        write8(buf, rom_off+0x310000 + 13, (uint8_t)slow-1); // +carry
        write8(buf, rom_off+0x310013 +  3, (uint8_t)slow);
        
        printf("Updating alchemy leveling...\n");
        uint8_t alchemy_exp[9];
        for (size_t i=0; i<ARRAY_SIZE(alchemy_exp); i++) {
            uint16_t exp = read8(buf, rom_off+0x045b9c+i);
            if (i>0 && exp == read8(buf, rom_off+0x045b9c+i-1)) {
                // same as previous -> floor
                exp = (exp * exp_num) / exp_den;
            } else if (i<8 && exp == read8(buf, rom_off+0x045b9c+i-1)) {
                // same as next -> ceil
                exp = (exp * exp_num + exp_den-1) / exp_den;
            } else {
                // default: round
                exp = (exp * exp_num + exp_den/2) / exp_den;
            }
            if (exp>255) exp=255;
            if (exp<1) exp=1;
            alchemy_exp[i] = (uint8_t)exp;
        }
        for (size_t i=0; i<ARRAY_SIZE(alchemy_exp); i++) {
            write8(buf, rom_off+0x045b9c+i, alchemy_exp[i]);
        }
    }

    if (placement_file) {
        // currently traps only exist for AP
        printf("Adding traps...\n");
        grow = true;
        APPLY_TRAPS();
    }
    if (placement_file || energy_core == ENERGY_CORE_FRAGMENTS) {
        printf("Adding energy core fragment...\n");
        grow = true;
        APPLY_ENERGY_CORE_FRAGMENT();
        if (energy_core == ENERGY_CORE_FRAGMENTS) {
            APPLY_ENERGY_CORE_FRAGMENT_HANDLING();
            (void)available_fragments;
            buf[rom_off + 0x3082ba] = required_fragments;
            buf[rom_off + 0x03273a] = required_fragments > 9 ? ('0' + (required_fragments / 10)) : ' ';
            buf[rom_off + 0x03273b] = '0' + (required_fragments % 10);
        }
    }

    if (randomized || energy_core == ENERGY_CORE_FRAGMENTS) {
        // ALCHEMY_RANDO_DROPS57 is required for fragments
        if (alchemizer) {
            printf("Applying alchemizer...\n");
        } else {
            printf("Fixing alchemists...\n");
        }

        grow = true;
        for (uint8_t i=0; i<ALCHEMY_COUNT; i++) {
            // Replace alchemy flags by location flags
            // new flags at $7e2570..74
            uint16_t new_flag = ((0x2570 - 0x2258)<<3) + alchemy_locations[i].id;
            for (size_t j=0; alchemy_locations[i].locations[j] != LOC_END; j++) {
                size_t loc = alchemy_locations[i].locations[j];
                buf[rom_off + loc + 0] = new_flag & 0xff;
                buf[rom_off + loc + 1] = new_flag >> 8;
            }
            // Insert item setup code (we jump there from whatever gives us the formula)
            // setups at 0xb0813b + 0x06*ID
            uint32_t setup_dst = (0xb0813b-0x800000) + 6 * alchemy_locations[i].id;
            uint8_t write_item[]   = { 0x17, 0x39, 0x01, 0x00, 0x00, 0x00 }; // set item id, end
            write_item[3] = alchemy_locations[i].id & 0xff;
            write_item[4] = 0x10;
            memcpy(buf + rom_off + setup_dst  + 0, write_item, sizeof(write_item));
            // Insert item reward code (we jump there from vanilla alchemy code)
            // drops at 0xb08000 + 0x09*ID
            uint32_t drop_dst  = (0xb08000-0x800000) + 9 * alchemy_locations[i].id;
            uint8_t call_setup[]   = { 0x29, 0x00, 0x00, 0x00 };       // jump target sets item id and amount
            uint8_t call_drop[]    = { 0x29, 0x0d, 0x02, 0x0f, 0x00 }; // jump target actually gives reward, end
            uint32_t setup_tgt = get_drop_setup_target_from_packed(alchemy[i]);
            call_setup[1] = (uint8_t)(setup_tgt>>0);
            call_setup[2] = (uint8_t)(setup_tgt>>8);
            call_setup[3] = (uint8_t)(setup_tgt>>16);
            memcpy(buf + rom_off + drop_dst + 0, call_setup, sizeof(call_setup));
            memcpy(buf + rom_off + drop_dst + 4, call_drop,  sizeof(call_drop));
        }

        // Make alchemy become items
        APPLY_ALCHEMY_TEXTS();
        APPLY_ALCHEMY_RANDO_DROPS(); // this has to be applied after changing the alchemy flags
    }

    if (ingredienizer) {
        _Static_assert(sizeof(*ingredients)==4, "Bad padding"); // required for memcpy
        printf("Applying ingredienizer...\n");
        for (uint8_t i=0; i<ALCHEMY_COUNT; i++) {
            uint16_t id = alchemy_locations[i].id;
            memcpy(buf + rom_off + 0x4601F + id*4, &(ingredients[i]), 4);
        }
    }
    if (bossdropamizer || placement_file) {
        printf("Applying fixes for randomized boss drops...\n");
        APPLY(74); APPLY(78); APPLY(79); APPLY(80);
        // v015:
        APPLY(78a); APPLY(78b); APPLY(78c); APPLY(78d); APPLY(78e);
        APPLY(143); APPLY(144);
        if (!placement_file && !gourdomizer) {
            // v026: swap Halls SW Wings with Halls NE Wax
            APPLY(HALLS_WINGS);  APPLY(HALLS_WINGS2);
            APPLY(HALLS_WINGS3); APPLY(HALLS_WINGS4);
            APPLY(HALLS_WINGS5); APPLY(HALLS_WINGS6);
            APPLY(HALLS_WINGS7); APPLY(HALLS_WINGS8);
        }
        printf("Applying boss dropamizer...\n");
        APPLY(77);
        APPLY(81);  /*APPLY(82);*/
        APPLY_BOSS_RANDO_DROPS();
        APPLY(83);  APPLY(84);  APPLY(85);  APPLY(86);
        APPLY(87);  APPLY(88);  APPLY(89);  APPLY(90);  APPLY(91);  APPLY(92);
        APPLY(93);  APPLY(94);  APPLY(95);  APPLY(96);  APPLY(97);  APPLY(98);
        APPLY(99);  APPLY(100); APPLY(101); APPLY(102); APPLY(103); APPLY(104);
        APPLY(105); APPLY(105b);
        // actually apply boss drop randomization
        for (size_t i=0; i<ARRAY_SIZE(boss_drops); i++) {
            uint32_t drop_dst = boss_drop_setup_jumps[i] - 1;
            // convert from rom to script addr
            drop_dst = (drop_dst & 0x7fff) | ((((drop_dst&0x7fffff)-0x120000) >> 1) & 0xff8000);
            buf[rom_off + boss_drop_jumps[i] + 0] = (uint8_t)(drop_dst>>0)&0xff;
            buf[rom_off + boss_drop_jumps[i] + 1] = (uint8_t)(drop_dst>>8)&0xff;
            buf[rom_off + boss_drop_jumps[i] + 2] = (uint8_t)(drop_dst>>16)&0xff;
            uint32_t setup_dst = get_drop_setup_target_from_packed(boss_drops[i]);
            buf[rom_off + boss_drop_setup_jumps[i] + 0] = (uint8_t)(setup_dst>>0)&0xff;
            buf[rom_off + boss_drop_setup_jumps[i] + 1] = (uint8_t)(setup_dst>>8)&0xff;
            buf[rom_off + boss_drop_setup_jumps[i] + 2] = (uint8_t)(setup_dst>>16)&0xff;
        }
    }
    if (randomized || energy_core == ENERGY_CORE_FRAGMENTS) {
        printf("Adding new item IDs...\n");
        grow = true;
        APPLY_GOURDOMIZER_DROPS();
    }
    if (placement_file || gourdomizer || energy_core == ENERGY_CORE_FRAGMENTS) {
        // v023:
        printf("Applying fixes for randomized gourds...\n");
        APPLY(REVERSE_BBM); APPLY(REVERSE_BBM2); APPLY(REVERSE_BBM3);
        APPLY(GOURDOMIZER_FIXES); APPLY(GOURDOMIZER_FIXES2);
        APPLY(GOURDOMIZER_FIXES3);
        // v029: make act3 chests unmissable
        APPLY(GOURDOMIZER_FIXES4);
        // v026: re-enter colosseum
        APPLY(COLOSSEUM);  APPLY(COLOSSEUM2);
        APPLY(COLOSSEUM3); APPLY(COLOSSEUM4);
        APPLY(COLOSSEUM5); APPLY(COLOSSEUM6);
        printf("Applying gourdomizer...\n");
        for (size_t i=0; i<ARRAY_SIZE(gourd_drops_data); i++) {
            const struct gourd_drop_item* d = &(gourd_drops_data[i]);
            memcpy(buf + rom_off + d->pos, d->data, d->len);
        }
        for (size_t i=0; i<ARRAY_SIZE(gourd_data); i++) {
            const struct gourd_data_item* g = &(gourd_data[i]);
            uint32_t setup_addr = get_drop_setup_target_from_packed(gourd_drops[i]);
            memcpy(buf + rom_off + g->pos, g->data, g->len);
            buf[rom_off + g->call_target_addr+0] = (setup_addr>> 0) & 0xff;
            buf[rom_off + g->call_target_addr+1] = (setup_addr>> 8) & 0xff;
            buf[rom_off + g->call_target_addr+2] = (setup_addr>>16) & 0xff;
        }
        APPLY_ENERGY_CORE_FIX();
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
        APPLY(JUKEBOX_NOBILIAFP);
        APPLY(JUKEBOX_STRONGHHE);
        APPLY(JUKEBOK_SWAMPPEPPER);
        APPLY(JUKEBOK_SWAMPSLEEP);
    }
    if (sniffamizer == POOL) {
        printf("Applying sniffamizer...\n");
        if (!openworld) // already applied as part of open world
            APPLY(MUD_PEPPER_LIMIT);
        APPLY_BBM_RESPAWN_BRIDGES(); // otherwise some would be missable
        APPLY_HORACE_CAMP_FIX_SNIFF(); // avoid getting stuck, needs extra rom
        // create setup scripts for all possible sniff drops
        for (uint16_t item=0x200; item<0x217; item++) {
            size_t addr = 0x30ff00 + 6 * (item - 0x200);
            if (item == 0x216)
                item = 0x41f; // iron bracer
            const uint8_t script[] = {
                0x17, 0x39, 0x01, (uint8_t)(item & 0xff), (uint8_t)(item >> 8), 0
            };
            memcpy(buf + rom_off + addr, script, sizeof(script));
        }
        // jump to setup script for each sniff location
        for (size_t i=0; i<ARRAY_SIZE(sniff_drops); i++) {
            uint32_t setup_addr = get_drop_setup_target_from_packed(sniff_drops[i]);
            assert(sniff_data[i].call_length >= 4);
            assert(setup_addr);
            buf[rom_off + sniff_data[i].call_pos + 0] = 0x07;
            buf[rom_off + sniff_data[i].call_pos + 1] = (setup_addr>> 0) & 0xff;
            buf[rom_off + sniff_data[i].call_pos + 2] = (setup_addr>> 8) & 0xff;
            buf[rom_off + sniff_data[i].call_pos + 3] = (setup_addr>>16) & 0xff;
            for (size_t n=4; n<sniff_data[i].call_length; n++)
                buf[rom_off + sniff_data[i].call_pos + n] =  0x4d; // NOP
        }
    }
    else if (sniffamizer || sniffingredients) {
        printf("Applying sniffamizer...\n");
        if (!openworld) // already applied as part of open world
            APPLY(MUD_PEPPER_LIMIT);
        for (size_t i=0; i<ARRAY_SIZE(sniff_drops); i++) {
            // sniff encodes in-game item id in the lower 10 bits
            uint16_t ingredient = sniff_drops[i] & 0x3ff;
            buf[rom_off + sniff_data[i].item_pos + 0] = (uint8_t)(ingredient & 0xff);
            buf[rom_off + sniff_data[i].item_pos + 1] = (uint8_t)(ingredient >> 8);
        }
    }
    if (callbeadamizer) {
        printf("Applying callbeadamizer...\n");
        for (size_t i=0; i<ARRAY_SIZE(callbead_menus); i++) {
            write8(buf, rom_off+callbead_menu_action_addrs[i], callbead_menus[i]);
        }
        if (callbeadamizer==FULL) {
            for (size_t i=0; i<ARRAY_SIZE(callbead_spells); i++) {
                write16(buf, rom_off+callbead_spell_item_addrs[i], callbead_spells[i]);
            }
        }
    }
    if (doggomizer || pupdunk) {
        printf("Applying doggomizer...\n");
        for (size_t i=0; i<ARRAY_SIZE(doggo_changes); i++) {
            if ((doggos[i].flags & DOGGO_FLAG_HARD) && difficulty<2) continue; // skip
            if ((doggos[i].flags & DOGGO_FLAG_OPENWORLD) && !openworld) continue; // skip
            buf[rom_off + doggos[i].addr] = doggo_changes[i];
        }
        // fix up act0 doggo
        APPLY(ACT0_DOG);
        APPLY(ACT0_DOG2);
    }

    if (shortbossrush) {
        printf("Shortening boss rush...\n");
        APPLY_SHORT_BOSS_RUSH();
    }
    if (shortdialogs) {
        printf("Cutting cutscenes...\n");
        APPLY_CUT_CUTSCENES();
    }
    if (turdomode) {
        printf("Applying turd...\n");
        APPLY_UNIVERSAL_HARD_BALL(); // patch Hard Ball to work with any spell number
        APPLY_ALL_HARD_BALL(); // convert all offensive spells to Hard Ball
        APPLY_TURD_BALL(); // convert Hard Ball to Turd Ball
        APPLY_ALTERNATIVE_TURD_BALLS(); // change description of stronger variants
        APPLY_TURDO_BALANCING(); // rebalance game
        if (alchemizer || placement_file) {
            APPLY_TURDO_ALCHEMY_TEXTS(); // change naming of alchemy items
        }
    }

    printf("Increasing save file size...\n");
    APPLY_SAVE_FILE_GROW();

    grow = true;

    if (!mystery) {
        for (uint8_t n = 0; n < 8; n++)
            buf[rom_off + seed_loc + n] = (seed >> (n * 8)) & 0xff;
    }

    memcpy(buf + rom_off + version_loc, &VERSION[1], 3);
    buf[rom_off + version_loc + 3] = 0; // reserved

#ifdef WITH_MULTIWORLD
    if (id_data_set) {
        printf("Applying id data...\n");
        memcpy(buf + rom_off + id_loc, id_data, sizeof(id_data));
        // TODO: patch in memcpy to WRAM
        printf("Applying multiworld hook...\n");
        APPLY_MULTIWORLD();
        if (death_link) {
            printf("Enabling death link...\n");
            buf[rom_off + flags_loc] |= 0x80;
        }
    }
#endif

    if (mystery) {
        buf[rom_off + flags_loc] |= 0x01; // tell tracker it shouldn't spoil
    }

    // if check value differs, the generated ROMs are different.
    uint64_t seedcheck = (uint16_t)(rand64()&0x3ff); // 10bits=2 b32 chars
    uint64_t curflag = 0x400;
    for (size_t i=0; i<ARRAY_SIZE(options); i++) {
        if (options[i].key == 'm') continue;
        size_t n=0;
        for (;options[i].state_names[n]; n++);
        if (n) {
            seedcheck |= curflag * option_values[i];
            for (;n>1;n=(n+1)/2) curflag <<= 1;
        }
    }
    seedcheck |= curflag * difficulty;
    char sseedcheckbuf[14] = {0}; // ceil(64/5)+1
    for (uint8_t i=0, j=0; i<64; i+=5) sseedcheckbuf[j++] = b32(seedcheck>>(60-i));
    const char* sseedcheck = sseedcheckbuf;
    while (sseedcheck[0] == 'a' && sseedcheck[1]) sseedcheck++;
    printf("\nCheck: %s (Please compare before racing)\n", sseedcheck);

    char shortsettings[sizeof(settings)] = {0};
    {
        char* a = shortsettings; char* b = settings;
        while (*b) if (*b!='r' && *b!='l') *a++=*b++; else b++;
        if (!shortsettings[0]) shortsettings[0]='r';
        assert(shortsettings[ARRAY_SIZE(shortsettings) - 1] == 0);
    }

    _Static_assert(sizeof(shortsettings) <= 35, "Too many options for space in ROM");
    if (!mystery) {
        memcpy(buf + rom_off + settings_loc, shortsettings, sizeof(shortsettings));
    }

    APPLY_MEMCPY_SETTINGS();

    size_t dsttitle_len = strlen("Evermizer_")+strlen("2P_")+strlen(VERSION)+1+sizeof(shortsettings)-1+1+16+1;
    char* dsttitle = (char*)malloc(dsttitle_len); // Evermizer_vXXX_e0123caibgsdm_XXXXXXXXXXXXXXXX
    int printlen = snprintf(dsttitle, dsttitle_len, "Evermizer_%s%s_%s_%" PRIx64, is_2p?"2P_":"", VERSION, shortsettings, seed);
    assert(printlen < (int)dsttitle_len);
    if (!randomized)
    {
        printlen = snprintf(dsttitle, dsttitle_len, "SoE-OpenWorld_%s%s_%s", is_2p?"2P_":"", VERSION, shortsettings);
        assert(printlen < (int)dsttitle_len);
        (void)printlen;
    }
    const char* pSlash = strrchr(src, DIRSEP);
    if (!pSlash && DIRSEP!='/') pSlash = strrchr(src, '/'); // wine support
    const char* ext = strrchr(src, '.');
    if (!ext || ext<pSlash) ext = ".sfc";
    size_t baselen = pSlash ? (pSlash-src+1) : 0;
    size_t dstbuf_len = dstdir? (strlen(dstdir)+1+strlen(dsttitle)+strlen(ext)) : (baselen+strlen(dsttitle)+strlen(ext))+1;
    char* dstbuf = (char*)malloc(dstbuf_len);
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
    
    // write ROM
    if (!dry) {
        FILE* fdst = fopen(dst,"wb");
        if (!fdst) { fclose(fsrc); free(buf); die("Could not open output file!\n"); }
        if (grow) sz+=GROW_BY;
        // TODO: recalculate checksum
        len = fwrite(buf, 1, sz, fdst);
        fclose(fdst); fdst=NULL;
        if (len<sz) { fclose(fsrc); free(buf); die("Could not write output file!\n"); }
        printf("Rom saved as %s !\n", dst);
    }
    
    // write spoiler log
    if (spoilerlog) {
    size_t logdstbuf_len = strlen(dst)+strlen("_SPOILER.log")+1;
    char* logdstbuf = (char*)malloc(logdstbuf_len);;
    pSlash = strrchr(dst, DIRSEP);
    ext = strrchr(dst, '.');
    if (!ext || ext<pSlash) ext = dst+strlen(dst);
    memcpy(logdstbuf, dst, ext-dst);
    memcpy(logdstbuf+(ext-dst), "_SPOILER.log", strlen("_SPOILER.log")+1);
    
    FILE* flog = fopen(logdstbuf,"wb");
    if (!flog) { fclose(fsrc); free(buf); die("Could not open spoiler log file!\n"); }
    #define ENDL "\r\n"
    fprintf(flog,"Spoiler log for evermizer %s settings %s seed %" PRIx64 "%s", VERSION, shortsettings, seed, ENDL);
    if (placement_file)
        fprintf(flog,"Placement from file" ENDL);
    else
        fprintf(flog,"Tree depth: %d, cyber logic score: %d, cyber gameplay score: %d%s", treedepth, cyberlogicscore, cybergameplayscore, ENDL);
    fprintf(flog,"Exp%%: %d, Money%%: %d%s", u8_fraction_to_percent(exp_num,exp_den), u8_fraction_to_percent(money_num,money_den), ENDL);
    fprintf(flog, ENDL);
    fprintf(flog,"     %-15s  %-18s   %-18s  %s" ENDL, "Spell", "Ingredient 1", "Ingredient 2", "Location");
    fprintf(flog,"------------------------------------------------------------------------" ENDL);
    for (size_t i=0; i<ALCHEMY_COUNT; i++) {
        const struct formula* f = &(ingredients[i]);

        const char* location = NULL;
        char name_buf[64];
        for (size_t j=0; j<ALCHEMY_COUNT; j++) {
            if (alchemy[j] == (CHECK_ALCHEMY<<10) + i) {
                location = alchemy_locations[j].name;
                break;
            }
        }
        if (!location) for (size_t j=0; j<ARRAY_SIZE(boss_drops); j++) {
            if (boss_drops[j] == (CHECK_ALCHEMY<<10) + i) {
                location = boss_names[j];
                break;
            }
        }
        if (!location) for (size_t j=0; j<ARRAY_SIZE(gourd_drops); j++) {
            if (gourd_drops[j] == (CHECK_ALCHEMY<<10) + i) {
                snprintf(name_buf, sizeof(name_buf), "%s Gourd #%u",
                        gourd_data[j].name, (unsigned)j);
                location = name_buf;
            }
        }
        if (!location) for (size_t j=0; j<ARRAY_SIZE(sniff_drops); j++) {
            if (sniff_drops[j] == (CHECK_ALCHEMY<<10) + i) {
                snprintf(name_buf, sizeof(name_buf), "%s Sniff #%u",
                        sniff_data[j].location_name, (unsigned)j);
                location = name_buf;
            }
        }
        if (!location) location = placement_file ? "Remote" : "Missing";

        fprintf(flog,"(%02d) %-15s  %dx %-15s + %dx %-15s  %s" ENDL,
            alchemy_locations[i].id, 
            alchemy_locations[i].name,
            f->amount1, ingredient_names[f->type1],
            f->amount2, ingredient_names[f->type2],
            location);
    }
    fprintf(flog,"------------------------------------------------------------------------" ENDL);
    fprintf(flog, ENDL);
    fprintf(flog,"     %-15s  %s" ENDL, "Boss", "Drop");
    fprintf(flog,"------------------------------------------------------------------------" ENDL);
    for (size_t i=0; i<ARRAY_SIZE(boss_drops); i++) {
        fprintf(flog,"(%02d) %-15s  %s" ENDL, (int)i, boss_names[i], get_drop_name_from_packed(boss_drops[i]));
    }
    fprintf(flog,"------------------------------------------------------------------------" ENDL);
    fprintf(flog, ENDL);
    fprintf(flog,"     %-15s  %s" ENDL, "Alchemist", "Drop");
    fprintf(flog,"------------------------------------------------------------------------" ENDL);
    for (size_t i=0; i<ALCHEMY_COUNT; i++) {
        fprintf(flog,"(%02d) %-15s  %s" ENDL, (int)i, alchemy_locations[i].name, get_drop_name_from_packed(alchemy[i]));
    }
    fprintf(flog,"------------------------------------------------------------------------" ENDL);
    if (placement_file || gourdomizer || energy_core == ENERGY_CORE_FRAGMENTS) {
    fprintf(flog, ENDL);
    fprintf(flog,"      %-19s  %s" ENDL, "Gourd", "Drop");
    fprintf(flog,"------------------------------------------------------------------------" ENDL);
    for (size_t i=0; i<ARRAY_SIZE(gourd_drops); i++) {
        // TODO: hide crap drops
        fprintf(flog,"(%03d) %-19s  %s" ENDL, (int)i, gourd_data[i].name, get_drop_name_from_packed(gourd_drops[i]));
    }
    fprintf(flog,"------------------------------------------------------------------------" ENDL);
    }
    if (sniffamizer == POOL) {
    fprintf(flog, ENDL);
    fprintf(flog,"      %-21s  %s" ENDL, "Sniff", "Drop");
    fprintf(flog,"------------------------------------------------------------------------" ENDL);
    for (size_t i=0; i<ARRAY_SIZE(sniff_drops); i++) {
        if (sniff_data[i].excluded)
            continue;
        // TODO: hide crap drops
        fprintf(flog,"(%03d) %-21s  %s" ENDL, (int)i, sniff_data[i].location_name, get_drop_name_from_packed(sniff_drops[i]));
    }
    fprintf(flog,"------------------------------------------------------------------------" ENDL);
    }
    #undef ENDL
    fclose(flog); flog=NULL;
    printf("Spoiler log saved as %s !\n", logdstbuf);
    free(logdstbuf);
    }

    free(dstbuf);
    free(dsttitle);
    free(buf);
    fclose(fsrc);
#if (defined(WIN32) || defined(_WIN32)) && !defined(NO_UI)
    if (!batch) pause();
#endif
    return 0;
}
