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

#endif // UTIL_H_INCLUDED
