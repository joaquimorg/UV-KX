// -----------------------------------------------------------------------------------------
// This file contains configuration settings for a lightweight printf library,
// often used in embedded systems to control code size by excluding unused features.
// Each #define directive in this file, when uncommented or defined, disables
// a specific feature or data type support in the printf implementation.
// -----------------------------------------------------------------------------------------

// Disables support for 'long long' integer types (e.g., %lld, %llu).
// This can save code space if 64-bit integer printing is not required.
#define PRINTF_DISABLE_SUPPORT_LONG_LONG

// Disables support for exponential notation in floating-point numbers (e.g., %e, %E).
// Useful if floating-point numbers are used but exponential format is not needed.
#define PRINTF_DISABLE_SUPPORT_EXPONENTIAL

// Disables support for 'ptrdiff_t' type, typically used with the %t specifier.
// ptrdiff_t is the type of the result of subtracting two pointers.
#define PRINTF_DISABLE_SUPPORT_PTRDIFF_T

// Disables all floating-point number support (e.g., %f, %g).
// This provides significant code size reduction if floating-point printing is not necessary.
#define PRINTF_DISABLE_SUPPORT_FLOAT
