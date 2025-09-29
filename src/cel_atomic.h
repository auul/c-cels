#ifndef CEL_ATOMIC_H
#define CEL_ATOMIC_H

#include "cel_core.h"

/* Configuration Knobs */

#ifndef CEL_ATOMIC_MUTEX_STRIPES
#	define CEL_ATOMIC_MUTEX_STRIPES 64
#endif

/* Determine Atomics Availability */

#define CEL_ATOMIC_STDATOMIC 8
#define CEL_ATOMIC_GNUC 7
#define CEL_ATOMIC_SYNC 6
#define CEL_ATOMIC_WIN_ARM64 5
#define CEL_ATOMIC_WIN_ARM32 4
#define CEL_ATOMIC_WIN_X64 3
#define CEL_ATOMIC_WIN_X86 2
#define CEL_ATOMIC_MUTEX 1

#if CEL_STDC >= CEL_STDC11 && !defined(__STDC_NO_ATOMICS__)
#	include <stdatomic.h>
#	define CEL_ATOMIC_USE CEL_ATOMIC_STDATOMIC
#	define CEL_ATOMIC_USE_TOKEN CEL_ATOMIC_STDATOMIC_
#elif defined(__clang__) && defined(__has_builtin) &&                          \
	__has_builtin(__atomic_load_n)
#	define CEL_ATOMIC_USE CEL_ATOMIC_GNUC
#	define CEL_ATOMIC_USE_TOKEN CEL_ATOMIC_GNUC_
#elif defined(_MSC_VER)
#	include <intrin.h>

#	if defined(M_ARM64)
#		define CEL_ATOMIC_USE CEL_ATOMIC_WIN_ARM64
#		define CEL_ATOMIC_USE_TOKEN CEL_ATOMIC_WIN_ARM64_
#	elif defined(M_ARM)
#		define CEL_ATOMIC_USE CEL_ATOMIC_WIN_ARM32
#		define CEL_ATOMIC_USE_TKEN CEL_ATOMIC_WIN_ARM32_
#	elif defined(_M_X64)
#		include <emmintrin.h>
#		define CEL_ATOMIC_USE CEL_ATOMIC_WIN_X64
#		define CEL_ATOMIC_USE_TOKEN CEL_ATOMIC_WIN_X64_
#	elif defined(_M_IX86)
#		include <emmintrin.h>
#		define CEL_ATOMIC_USE CEL_ATOMIC_WIN_X86
#		define CEL_ATOMIC_USE_TOKEN CEL_ATOMIC_WIN_X86_
#	endif
#elif CEL_GNUC >= 40700
#	define CEL_ATOMIC_USE CEL_ATOMIC_GNUC
#	define CEL_ATOMIC_USE_TOKEN CEL_ATOMIC_GNUC_
#elif CEL_GNUC >= 40100
#	define CEL_ATOMIC_USE CEL_ATOMIC_SYNC
#	define CEL_ATOMIC_USE_TOKEN CEL_ATOMIC_SYNC_
#elif defined(__unix__) || defined(__unix) || defined(unix) ||                 \
	defined(__APPLE__) || defined(__MACH__) || defined(__linux__) ||           \
	defined(__ANDROID__) || defined(__CYGWIN__) || defined(__EMSCRIPTEN__) ||  \
	defined(__wasi__)
#	include <unistd.h>
#	if defined _POSIX_THREADS
#		include <pthread.h>
#		define CEL_ATOMIC_USE CEL_ATOMIC_MUTEX
#		define CEL_ATOMIC_USE_TOKEN CEL_ATOMIC_MUTEX_
#	endif
#endif

#if !defined(CEL_ATOMIC_USE)
#	define CEL_ATOMIC_USE 0
#endif

CEL_STATIC_ASSERT(
	CEL_ATOMIC_USE, "No available atomics or mutexes", no_available_atomics);

/* Integer Types */

#if CEL_ATOMIC_USE == CEL_ATOMIC_STDATOMIC
#	define CEL_ATOMIC_MOD() _Atomic
#elif CEL_ATOMIC_USE != CEL_ATOMIC_MUTEX
#	define CEL_ATOMIC_MOD() volatile
#else
#	define CEL_ATOMIC_MOD()
#endif

typedef CEL_ATOMIC_MOD() cel_u8 cel_atomic_u8;
typedef CEL_ATOMIC_MOD() cel_i8 cel_atomic_i8;
typedef CEL_ATOMIC_MOD() cel_u16 cel_atomic_u16;
typedef CEL_ATOMIC_MOD() cel_i16 cel_atomic_i16;
typedef CEL_ATOMIC_MOD() cel_u32 cel_atomic_u32;
typedef CEL_ATOMIC_MOD() cel_i32 cel_atomic_i32;
#if CEL_HAS_INT64
typedef CEL_ATOMIC_MOD() cel_u64 cel_atomic_u64;
typedef CEL_ATOMIC_MOD() cel_i64 cel_atomic_i64;
#endif
typedef CEL_ATOMIC_MOD() size_t cel_atomic_size;
typedef CEL_ATOMIC_MOD() cel_uptr cel_atomic_uptr;

/* Memory Ordering */

typedef enum {
	CEL_ATOMIC_RELAXED,
	CEL_ATOMIC_ACQUIRE,
	CEL_ATOMIC_RELEASE,
	CEL_ATOMIC_ACQ_REL,
	CEL_ATOMIC_SEQ_CST,
} cel_atomic_memorder;

#if CEL_ATOMIC_USE == CEL_ATOMIC_STDATOMIC
CEL_INLINE int cel_pvt_atomic_memorder(int memorder)
{
	switch (memorder) {
	case CEL_ATOMIC_RELAXED:
		return memory_order_relaxed;
	case CEL_ATOMIC_ACQUIRE:
		return memory_order_acquire;
	case CEL_ATOMIC_RELEASE:
		return memory_order_release;
	case CEL_ATOMIC_ACQ_REL:
		return memory_order_acq_rel;
	default:
		return memory_order_seq_cst;
	}
}
#elif CEL_ATOMIC_USE == CEL_ATOMIC_GNUC
CEL_INLINE int cel_pvt_atomic_memorder(int memorder)
{
	switch (memorder) {
	case CEL_ATOMIC_RELAXED:
		return __ATOMIC_RELAXED;
	case CEL_ATOMIC_ACQUIRE:
		return __ATOMIC_ACQUIRE;
	case CEL_ATOMIC_RELEASE:
		return __ATOMIC_RELEASE;
	case CEL_ATOMIC_ACQ_REL:
		return __ATOMIC_ACQ_REL;
	default:
		return __ATOMIC_SEQ_CST;
	}
}
#else
#	define cel_pvt_atomic_memorder(memorder) memorder
#endif

CEL_INLINE int cel_pvt_atomic_read_order(int memorder)
{
	switch (memorder) {
	case CEL_ATOMIC_RELEASE:
	case CEL_ATOMIC_ACQ_REL:
		return cel_pvt_atomic_memorder(CEL_ATOMIC_SEQ_CST);
	default:
		return cel_pvt_atomic_memorder(memorder);
	}
}

CEL_INLINE int cel_pvt_atomic_write_order(int memorder)
{
	switch (memorder) {
	case CEL_ATOMIC_ACQUIRE:
	case CEL_ATOMIC_ACQ_REL:
		return cel_pvt_atomic_memorder(CEL_ATOMIC_SEQ_CST);
	default:
		return cel_pvt_atomic_memorder(memorder);
	}
}

CEL_INLINE int cel_pvt_atomic_fail_order(int memorder)
{
	switch (memorder) {
	case CEL_ATOMIC_ACQUIRE:
	case CEL_ATOMIC_ACQ_REL:
		return cel_pvt_atomic_memorder(CEL_ATOMIC_ACQUIRE);
	default:
		return cel_pvt_atomic_memorder(CEL_ATOMIC_RELAXED);
	}
}

/* Mutex Boilerplate */

#if CEL_ATOMIC_USE == CEL_ATOMIC_MUTEX

CEL_STATIC_ASSERT(
	(CEL_ATOMIC_MUTEX_STRIPES & (CEL_ATOMIC_MUTEX_STRIPES - 1)) != 0,
	"CEL_ATOMIC_MUTEX_STRIPES must be a power of 2", stripes_not_pow2);

#	ifdef CEL_ATOMIC_IMPLEMENT
pthread_once_t cel_pvt_atomic_inited = PTHREAD_ONCE_INIT;
pthread_mutex_t cel_pvt_atomic_init_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cel_pvt_atomic_mutex[CEL_ATOMIC_MUTEX_STRIPES];

CEL_INLINE cel_u32 cel_pvt_atomic_mutex_index(const void *ptr)
{
#		if CEL_ATOMIC_MUTEX_STRIPES == 1
	return 0;
#		endif

	cel_uptr x = (cel_uptr)ptr >> 4;
#		if CEL_UPTR_SIZE == 8
	const cel_u64 k = CEL_U64(11400714819323198485);
	const unsigned w = 64;
	cel_u64 h = (cel_u64)x * k;
#		else
	const cel_u32 k = CEL_U32(2654435761);
	const unsigned w = 32;
	cel_u32 h = (cel_u32)x * k;
#		endif

	const cel_u32 kbits = CEL_LOG2_U32(CEL_ATOMIC_MUTEX_STRIPES);
	return (cel_u32)(h >> (w - kbits));
}

void cel_pvt_atomic_mutex_init(void)
{
	pthread_mutex_lock(&cel_pvt_atomic_init_mutex);
	if (cel_pvt_atomic_inited) {
		pthread_mutex_unlock(&cel_pvt_atomic_init_mutex);
		return;
	}

	cel_u32 i;
	for (i = 0; i < CEL_ATOMIC_MUTEX_STRIPES; i++) {
		pthread_mutex_init(&cel_pvt_atomic_mutex[i], NULL);
	}
	cel_pvt_atomic_inited = CEL_TRUE;

	pthread_mutex_unlock(&cel_pvt_atomic_init_mutex);
}

CEL_INLINE cel_u32 cel_pvt_atomic_mutex_lock(const void *ptr)
{
	pthread_once(&cel_pvt_pthread_inited, cel_pvt_atomic_mutex_init);
	cel_pvt_atomic_mutex_init();
	cel_u32 index = cel_pvt_atomic_mutex_index(ptr);
	pthread_mutex_lock(&cel_pvt_atomic_mutex[index]);

	return index;
}

CEL_INLINE void cel_pvt_atomic_mutex_unlock(cel_u32 index)
{
	pthread_mutex_unlock(&cel_pvt_atomic_mutex[index]);
}

#	endif
#endif

/* Helper Macros */

#define CEL_ATOMIC_API(type, fn)                                               \
	CEL_CAT3(CEL_CAT(cel_atomic_, type), CEL_CAT(_, fn), _explicit)

/* Stdatomic Templates */

#define CEL_ATOMIC_cas(type)                                                   \
	atomic_compare_exchange_strong_explicit(                                   \
		&expected, desired, cel_pvt_atomic_memorder(memorder),                 \
		cel_pvt_atomic_fail_order(memorder));                                  \
	return expected
#define CEL_ATOMIC_swap(type)                                                  \
	return atomic_exchange_explicit(                                           \
		ptr, value, cel_pvt_atomic_memorder(memorder))
#define CEL_ATOMIC_load(type)                                                  \
	return atomic_load_explicit(ptr, value, cel_pvt_atomic_read_order(memorder))
#define CEL_ATOMIC_store(type)                                                 \
	return atomic_store_explicit(                                              \
		ptr, value, cel_pvt_atomic_write_order(memorder))
#define CEL_ATOMIC_add(type)                                                   \
	return atomic_fetch_add_explicit(                                          \
		ptr, value, cel_pvt_atomic_memorder(memorder))
#define CEL_ATOMIC_sub(type)                                                   \
	return atomic_fetch_sub_explicit(                                          \
		ptr, value, cel_pvt_atomic_memorder(memorder))
#define CEL_ATOMIC_or(type)                                                    \
	return atomic_fetch_or_explicit(                                           \
		ptr, value, cel_pvt_atomic_memorder(memorder))
#define CEL_ATOMIC_and(type)                                                   \
	return atomic_fetch_and_explicit(                                          \
		ptr, value, cel_pvt_atomic_memorder(memorder))
#define CEL_ATOMIC_xor(type)                                                   \
	return atomic_fetch_xor_explicit(                                          \
		ptr, value, cel_pvt_atomic_memorder(memorder))

/* Mutex Templates */

#define CEL_ATOMIC_MUTEX_EXPR(type, expr)                                      \
	CEL_CAT(cel_, type) index = cel_pvt_atomic_mutex_lock(ptr);                \
	CEL_CAT(cel_, type) retval = *ptr;                                         \
	*ptr = (expr);                                                             \
	cel_pvt_atomic_mutex_unlock(index);                                        \
	return retval;

/* Fill-in Templates */

#define CEL_ATOMIC_CAS_LOOP(type, expr)                                        \
	CEL_CAT(cel_, type) expected, actual = 0;                                  \
	do {                                                                       \
		expected = actual;                                                     \
		actual = CEL_ATOMIC_API(type, cas)(ptr, expected, expr, memorder);     \
	} while (expected != actual);                                              \
	return actual;

#endif
