#ifndef CEL_ATOMIC_H
#define CEL_ATOMIC_H

#include "cel_core.h"

/* Determine Atomics Availability */

#define CEL_ATOMIC_STDATOMIC 8
#define CEL_ATOMIC_GNUC      7
#define CEL_ATOMIC_SYNC      6
#define CEL_ATOMIC_WIN10     5
#define CEL_ATOMIC_WIN8      4
#define CEL_ATOMIC_WIN7      3
#define CEL_ATOMIC_VISTA     2
#define CEL_ATOMIC_WINXP     1
#define CEL_ATOMIC_MUTEX     0

#if CEL_STDC >= 201112L && !defined(__STDC_NO_ATOMICS__)
#	include <stdatomic.h>
#	define CEL_ATOMIC_USE CEL_ATOMIC_STDATOMIC
#elif defined(__GNUC__) && __GNUC__ >= 4 && __GNUC_MINOR__ >= 1
#	if __GNUC__ >= 5 || __GNUC_MINOR__ >= 7
#		define CEL_ATOMIC_USE CEL_ATOMIC_GNUC
#	else
#		define CEL_ATOMIC_USE CEL_ATOMIC_SYNC
#	endif
#elif defined(_WIN32)
#	include <windows.h>
#	if _WIN32_WINNT >= _WIN32_WINNT_WIN10
#		define CEL_ATOMIC_USE CEL_ATOMIC_WIN10
#	elif _WIN32_WINNT >= _WIN32_WINNT_WIN8
#		define CEL_ATOMIC_USE CEL_ATOMIC_WIN8
#	elif _WIN32_WINNT >= _WIN32_WINNT_WIN7
#		define CEL_ATOMIC_USE CEL_ATOMIC_WIN7
#	elif _WIN32_WINNT >= _WIN32_WINNT_VISTA
#		define CEL_ATOMIC_USE CEL_ATOMIC_VISTA
#	elif _WIN32_WINNT >= _WIN32_WINNT_WINXP
#		define CEL_ATOMIC_USE CEL_ATOMIC_WINXP
#	endif
#elif defined(__unix__) || defined(__unix) || defined(unix)     \
	|| defined(__linux__) || defined(__linux) || defined(linux) \
	|| defined(__gnu_linux__) || defined(__APPLE__) || defined(__MACH__)
#	include <unistd.h>
#	if _POSIX_VERSION >= 199506L
#		define CEL_ATOMIC_USE CEL_ATOMIC_MUTEX
#	endif
#endif

#ifndef CEL_ATOMIC_USE
#	error "No atomic or lock primitives could be found"
#endif

#undef CEL_ATOMIC_USE
#undef CEL_ATOMIC_STDATOMIC
#undef CEL_ATOMIC_GNUC
#undef CEL_ATOMIC_SYNC
#undef CEL_ATOMIC_WIN10
#undef CEL_ATOMIC_WIN8
#undef CEL_ATOMIC_WIN7
#undef CEL_ATOMIC_VISTA
#undef CEL_ATOMIC_WINXP
#undef CEL_ATOMIC_MUTEX

#endif
