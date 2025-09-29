#ifndef CEL_CORE_H
#define CEL_CORE_H

#include <limits.h>
#include <stddef.h>

/* Preprocessor Macros */

#define CEL_CAT0(a, b) a##b
#define CEL_CAT(a, b) CEL_CAT0(a, b)
#define CEL_CAT3(a, b, c) CEL_CAT0(a, CEL_CAT0(a, b))

#define CEL_UNIQ(name) CEL_CAT(name, __LINE__)

#define CEL_APPLY(fn, args) CEL_EVAL1(fn args)

#define CEL_EVAL1(a) a
#define CEL_EVAL2(a) CEL_EVAL1(CEL_EVAL1(a))
#define CEL_EVAL4(a) CEL_EVAL2(CEL_EVAL2(a))
#define CEL_EVAL8(a) CEL_EVAL4(CEL_EVAL4(a))
#define CEL_EVAL(a) CEL_EVAL8(CEL_EVAL8(a))

/* C Version */

#define CEL_STDC89 0
#define CEL_STDC99 199901L
#define CEL_STDC11 201112L
#define CEL_STDC23 202311L

#ifdef __STDC_VERSION__
#	define CEL_STDC __STDC_VERSION__
#else
#	define CEL_STDC 0
#endif

#ifdef __cplusplus
#	define CEL_CPP __cplusplus
#else
#	define CEL_CPP 0
#endif

#ifdef __GNUC__
#	define CEL_GCC                                                            \
		(__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
#	define CEL_GCC 0
#endif

#ifdef __MSC_VER
#	define CEL_MSVC __MSC_VER
#else
#	define CEL_MSVC 0
#endif

/* Inline Keyword */

#if CEL_STDC >= CEL_STDC99
#	define CEL_INLINE static inline
#else
#	define CEL_INLINE static
#endif

/* Assertion Macros */

#if CEL_CPP >= 201103L || CEL_STDC >= CEL_STDC23
#	define CEL_STATIC_ASSERT(cond, str_msg, id_msg)                           \
		static_assert(cond, str_msg)
#elif CEL_STDC >= CEL_STDC11
#	define CEL_STATIC_ASSERT(cond, str_msg, id_msg)                           \
		_Static_assert(cond, str_msg)
#else
#	define CEL_STATIC_ASSERT(cond, str_msg, id_msg)                           \
		enum { CEL_UNIQ(cel_static_assert_##id_msg) = 1 / (!!(cond)) }
#endif

#define CEL_STATIC_ASSERT_EXPR(cond) sizeof(char[(cond) ? 1 : -1])

/* Flexible Array Member Macros */

#define CEL_FAM_ALIGN_PAD(parent, member, align)                               \
	(((align) - (offsetof(parent, member) % (align))) % align)

/* Booleans */

#if CEL_STDC >= CEL_STDC11 || CEL_CPP
#	if CEL_STDC >= CEL_STDC11
#		include <stdbool.h>
#	endif
typedef bool cel_bool;
#	define CEL_TRUE true
#	define CEL_FALSE false
#else
typedef int cel_bool;
#	define CEL_TRUE 1
#	define CEL_FALSE 0
#endif

/* Integer Types */

#if CEL_STDC >= CEL_STDC11
#	include <stdint.h>
typedef int_least8_t cel_i8;
typedef int_least16_t cel_i16;
typedef int_least32_t cel_i32;
typedef int_least64_t cel_i64;
typedef uint_least8_t cel_u8;
typedef uint_least16_t cel_u16;
typedef uint_least32_t cel_u32;
typedef uint_least64_t cel_u64;
#	define CEL_HAS_INT64 1
#else
typedef char cel_i8;
typedef short cel_i16;
typedef unsigned char cel_u8;
typedef unsigned short cel_u16;
#	if INT_MAX >= 0x7fffffff
typedef int cel_i32;
typedef unsigned cel_u32;
#	else
typedef long cel_i32;
typedef unsigned long cel_u32;
#	endif
#	if CEL_STDC >= CEL_STDC99 || CEL_GCC >= 20900
typedef long long cel_i64;
typedef unsigned long long cel_u64;
#		define CEL_HAS_INT64 1
#	elif CEL_MSVC >= 1200
typedef __int64 cel_i64;
typedef unsigned __int64 cel_u64;
#		define CEL_HAS_INT64 1
#	else
#		define CEL_HAS_INT64 0
#	endif
#endif

#if defined(UINTPTR_MAX) || defined(__UINTPTR_TYPE__)
typedef uintptr_t cel_uptr;
#	if UINTPTR_MAX == 0xfffffffful
#		define CEL_UPTR_SIZE 4
#	else
#		define CEL_UPTR_SIZE 8
#	endif
#	define CEL_HAS_UPTR 1
#elif defined(__SIZEOF_POINTER__)
#	if __SIZEOF_POINTER__ > 8
#		define CEL_HAS_UPTR 0
#	elif __SIZEOF_POINTER__ > 4
#		if CEL_HAS_INT64
typedef cel_u64 cel_uptr;
#			define CEL_HAS_UPTR 1
#		else
#			define CEL_HAS_UPTR 0
#		endif
#	else
typedef cel_u32 cel_uptr;
#	endif
#	define CEL_UPTR_SIZE __SIZEOF_POINTER__
#elif defined(_WIN64)
typedef cel_u64 cel_uptr;
#	define CEL_UPTR_SIZE 8
#elif defined(_WIN32)
typedef cel_u32 cel_uptr;
#	define CEL_UPTR_SIZE 4
#elif CEL_HAS_INT64
typedef cel_u64 cel_uptr;
#	define CEL_UPTR_SIZE 8
#else
typedef cel_u32 cel_uptr;
#	define CEL_UPTR_SIZE 4
#endif

CEL_STATIC_ASSERT(
	(CEL_HAS_UPTR), "No pointer-sized integer type found", no_uintptr_t);

#if CEL_HAS_INT64
#	define CEL_U64(n) n##ull
#	define CEL_I64(n) n##ll
#endif
#define CEL_U32(n) n##ul
#define CEL_I32(n) n##l

/* Alignment Macros */

#if CEL_CPP >= 201103L || CEL_STDC >= CEL_STDC23
#	define CEL_ALIGNOF(T) alignof(T)
#elif CEL_STDC >= CEL_STDC11
#	define CEL_ALIGNOF(T) _Alignof(T)
#elif CEL_GCC >= 20900
#	define CEL_ALIGNOF(T) __alignof__(T)
#elif CEL_MSVC >= 1310
#	define CEL_ALIGNOF(T) __alignof(T)
#else
#	define CEL_ALIGNOF(T)                                                     \
		offsetof(                                                              \
			struct {                                                           \
				char dummy;                                                    \
				T x;                                                           \
			},                                                                 \
			x)
#endif

#if CEL_STDC >= CEL_STDC11
#	include <stdalign.h>
typedef max_align_t cel_max_align;
#else
typedef long double cel_max_align;
#endif
#define CEL_MAX_ALIGN CEL_ALIGNOF(cel_max_align)

/* Mathematical Macros */

#define CEL_LOG2_U32(x)                                                        \
	(((x) & (1ul << 31))   ? 31                                                \
	 : ((x) & (1ul << 30)) ? 30                                                \
	 : ((x) & (1ul << 29)) ? 29                                                \
	 : ((x) & (1ul << 28)) ? 28                                                \
	 : ((x) & (1ul << 27)) ? 27                                                \
	 : ((x) & (1ul << 26)) ? 26                                                \
	 : ((x) & (1ul << 25)) ? 25                                                \
	 : ((x) & (1ul << 24)) ? 24                                                \
	 : ((x) & (1ul << 23)) ? 23                                                \
	 : ((x) & (1ul << 22)) ? 22                                                \
	 : ((x) & (1ul << 21)) ? 21                                                \
	 : ((x) & (1ul << 20)) ? 20                                                \
	 : ((x) & (1ul << 19)) ? 19                                                \
	 : ((x) & (1ul << 18)) ? 18                                                \
	 : ((x) & (1ul << 17)) ? 17                                                \
	 : ((x) & (1ul << 16)) ? 16                                                \
	 : ((x) & (1ul << 15)) ? 15                                                \
	 : ((x) & (1ul << 14)) ? 14                                                \
	 : ((x) & (1ul << 13)) ? 13                                                \
	 : ((x) & (1ul << 12)) ? 12                                                \
	 : ((x) & (1ul << 11)) ? 11                                                \
	 : ((x) & (1ul << 10)) ? 10                                                \
	 : ((x) & (1ul << 9))  ? 9                                                 \
	 : ((x) & (1ul << 8))  ? 8                                                 \
	 : ((x) & (1ul << 7))  ? 7                                                 \
	 : ((x) & (1ul << 6))  ? 6                                                 \
	 : ((x) & (1ul << 5))  ? 5                                                 \
	 : ((x) & (1ul << 4))  ? 4                                                 \
	 : ((x) & (1ul << 3))  ? 3                                                 \
	 : ((x) & (1ul << 2))  ? 2                                                 \
	 : ((x) & (1ul << 1))  ? 1                                                 \
						   : 0)

#endif
