#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

// Utility functions
#ifdef __cplusplus
#define _Static_assert static_assert
#endif
#ifdef WITH_ASSERT // include or override assert
#include <assert.h>
#else
#ifndef NO_ASSERT
#define NO_ASSERT
#pragma message "NOTE: Defaulting to NO_ASSERT"
#endif
#define assert(x) do { if(x){} } while(false);
#endif

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef SWAP
#define SWAP(a,b,T) do {\
    T SWAP = a;\
    a = b;\
    b = SWAP;\
} while(0)
#endif
#if (!defined(__STDC_VERSION__) || (__STDC_VERSION__ < 201112L)) && !defined(_Static_assert)
#   define _Static_assert(a,b)
#endif
#if defined __GNUC__ && !defined __clang__ && __GNUC__>=9 && defined __OPTIMIZE__
#   define GCC_Static_assert _Static_assert // non-standard static assert
#else
#   define GCC_Static_assert(a,b) assert(a) // fall back to runtime assert
#endif
#ifndef __has_builtin
  #define __has_builtin(x) 0
#endif
#if defined __GNUC__ && !defined __STRICT_ANSI__ && (__GNUC__>5 || __has_builtin(__builtin_types_compatible_p))
#define BUILD_BUG_OR_ZERO(e) ((int)(sizeof(struct { int:(-!!(e)); })))
#define ZERO_IF_ARRAY(a) BUILD_BUG_OR_ZERO(__builtin_types_compatible_p(typeof(a), typeof(&a[0])))
#else
#define ZERO_IF_ARRAY(a) 0
#endif
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]) + ZERO_IF_ARRAY(a))


static uint8_t read8(void* buf, size_t pos) {
    uint8_t* data = (uint8_t*)buf + pos;
    return data[0];
}
static uint16_t read16(void* buf, size_t pos) {
    uint8_t* data = (uint8_t*)buf + pos;
    uint16_t res = 0;
    res |= data[1]; res<<=8;
    res |= data[0];
    return res;
}
static uint32_t read32(void* buf, size_t pos) {
    uint8_t* data = (uint8_t*)buf + pos;
    uint32_t res = 0;
    res |= data[3]; res<<=8;
    res |= data[2]; res<<=8;
    res |= data[1]; res<<=8;
    res |= data[0];
    return res;
}
static void write8(void* buf, size_t pos, uint8_t val) {
    uint8_t* data = (uint8_t*)buf + pos;
    data[0] = val;
}
static void write16(void* buf, size_t pos, uint16_t val) {
    uint8_t* data = (uint8_t*)buf + pos;
    data[0] = val&0xff; val>>=8;
    data[1] = val&0xff;
}
static void write32(void* buf, size_t pos, uint32_t val) {
    uint8_t* data = (uint8_t*)buf + pos;
    data[0] = val&0xff; val>>=8;
    data[1] = val&0xff; val>>=8;
    data[2] = val&0xff; val>>=8;
    data[3] = val&0xff;
}

static bool percent_to_u8_fraction(int percent, uint8_t* num, uint8_t* den)
{
    if (percent<0) return false;
    if (percent<=255) {
        *num = (uint8_t)percent;
        *den = 100;
    } else if (percent<=2550) {
        *num = (uint8_t)((percent+5)/10);
        *den = 10;
    } else {
        return false;
    }
    return true;
}
int u8_fraction_to_percent(uint8_t num, uint8_t den)
{
    if (num == den || den == 0) return 100;
    return (int)num * 100 / den;
}

#endif // UTIL_H_INCLUDED
