#pragma once

#include <cstdint>
#include <U8g2lib.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

#ifndef ARRAY_SIZE_ELEMENT
#define ARRAY_SIZE_ELEMENT(x) (sizeof(x[0]))
#endif

/*
#ifndef MAX
    #define MAX(a, b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })
#endif

#ifndef MIN
    #define MIN(a, b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })
#endif

#ifndef SWAP
    #define SWAP(a, b) ({ __typeof__ (a) _c = (a);  a = b; b = _c; })
#endif
*/

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
