// -----------------------------------------------------------------------------------------
// This file provides miscellaneous utility macros and potentially small helper functions
// that are used across various parts of the firmware. These utilities help in
// reducing code duplication and improving readability.
// -----------------------------------------------------------------------------------------
#pragma once

#include <cstdint>   // For standard integer types.
#include <U8g2lib.h> // Included, but not directly used by the macros in this snippet.
                     // Might be used by other parts of misc.h not shown, or for type compatibility.

/**
 * @brief Calculates the number of elements in a statically allocated array.
 * @param x The array.
 * @return The number of elements in the array.
 * Example: `int myArray[10]; size_t s = ARRAY_SIZE(myArray); // s will be 10`
 */
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

/**
 * @brief Calculates the size of a single element in a statically allocated array.
 * @param x The array.
 * @return The size of an element in the array, in bytes.
 * Example: `int myArray[10]; size_t elem_s = ARRAY_SIZE_ELEMENT(myArray); // elem_s will be sizeof(int)`
 */
#ifndef ARRAY_SIZE_ELEMENT
#define ARRAY_SIZE_ELEMENT(x) (sizeof(x[0]))
#endif

/*
  The following are type-safe versions of MAX, MIN, and SWAP using GCC extensions
  (__typeof__). They are commented out, possibly due to compatibility concerns
  or preference for the simpler, less safe versions defined below.

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

/**
 * @brief Macro to find the maximum of two values.
 * Note: This version can have side effects if `a` or `b` are expressions
 * with side effects (e.g., `MAX(i++, j++)`) because `a` and `b` might be
 * evaluated multiple times.
 * @param a The first value.
 * @param b The second value.
 * @return The greater of `a` and `b`.
 */
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

/**
 * @brief Macro to find the minimum of two values.
 * Note: This version can have side effects if `a` or `b` are expressions
 * with side effects (e.g., `MIN(i++, j++)`) because `a` and `b` might be
 * evaluated multiple times.
 * @param a The first value.
 * @param b The second value.
 * @return The lesser of `a` and `b`.
 */
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
