#ifndef CEL_CORE_H
#define CEL_CORE_H

/* Platform Detection */

#ifdef __STDC_VERSION__
#	define CEL_STDC __STDC_VERSION__
#else
#	define CEL_STDC 0L
#endif

/* Shims */

#define CEL__CAT(a, b) a##b
#define CEL_CAT(a, b)  CEL__CAT(a, b)

#if CEL_STDC >= 199901L
#	define CEL_INLINE   static inline
#	define CEL_RESTRICT restrict
#else
#	define CEL_INLINE static
#	define CEL_RESTRICT
#endif

#if CEL_STDC >= 201112L
#	define CEL_ALIGNOF(type) _Alignof(type)
#else
#	define CEL_ALIGNOF(type)                       \
		offsetof(                                   \
			struct CEL_CAT(cel__##type, __LINE__) { \
				char c;                             \
				type t;                             \
			},                                      \
			t)
#endif

#endif
