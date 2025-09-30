#ifndef CEL_ATOMIC_H
#define CEL_ATOMIC_H

#include "cel_core.h"

/* Configuration Knobs */

#ifndef CEL_ATOMIC_MUTEX_STRIPES
#	define CEL_ATOMIC_MUTEX_STRIPES 64
#endif

/* Determine Atomics Availability */

#define CEL_ATOMIC_STD 8
#define CEL_ATOMIC_GNUC 7
#define CEL_ATOMIC_SYNC 6
#define CEL_ATOMIC_WIN_ARM64 5
#define CEL_ATOMIC_WIN_ARM32 4
#define CEL_ATOMIC_WIN_X64 3
#define CEL_ATOMIC_WIN_X86 2
#define CEL_ATOMIC_MUTEX 1

#if CEL_STDC >= CEL_STDC11 && !defined(__STDC_NO_ATOMICS__)
#	include <stdatomic.h>
#	define CEL_ATOMIC_USE CEL_ATOMIC_STD
#elif defined(__clang__) && defined(__has_builtin) &&                          \
	__has_builtin(__atomic_load_n)
#	define CEL_ATOMIC_USE CEL_ATOMIC_GNUC
#elif defined(_MSC_VER)
#	include <intrin.h>

#	if defined(_M_ARM64)
#		define CEL_ATOMIC_USE CEL_ATOMIC_WIN_ARM64
#	elif defined(_M_ARM)
#		define CEL_ATOMIC_USE CEL_ATOMIC_WIN_ARM32
#	elif defined(_M_X64)
#		include <emmintrin.h>
#		define CEL_ATOMIC_USE CEL_ATOMIC_WIN_X64
#	elif defined(_M_IX86)
#		include <emmintrin.h>
#		define CEL_ATOMIC_USE CEL_ATOMIC_WIN_X86
#	endif
#elif CEL_GNUC >= 40700
#	define CEL_ATOMIC_USE CEL_ATOMIC_GNUC
#elif CEL_GNUC >= 40100
#	define CEL_ATOMIC_USE CEL_ATOMIC_SYNC
#elif defined(__unix__) || defined(__unix) || defined(unix) ||                 \
	defined(__APPLE__) || defined(__MACH__) || defined(__linux__) ||           \
	defined(__ANDROID__) || defined(__CYGWIN__) || defined(__EMSCRIPTEN__) ||  \
	defined(__wasi__)
#	include <unistd.h>
#	if defined(_POSIX_THREADS)
#		include <pthread.h>
#		define CEL_ATOMIC_USE CEL_ATOMIC_MUTEX
#	endif
#endif

#if !defined(CEL_ATOMIC_USE)
#	define CEL_ATOMIC_USE 0
#endif

CEL_STATIC_ASSERT(
	CEL_ATOMIC_USE, "No available atomics or mutexes", no_available_atomics);

/* Integer Types */

#if CEL_ATOMIC_USE == CEL_ATOMIC_STD
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

#if CEL_ATOMIC_USE == CEL_ATOMIC_STD
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
		return CEL_ATOMIC_SEQ_CST;
	default:
		return memorder;
	}
}

CEL_INLINE int cel_pvt_atomic_write_order(int memorder)
{
	switch (memorder) {
	case CEL_ATOMIC_ACQUIRE:
	case CEL_ATOMIC_ACQ_REL:
		return CEL_ATOMIC_SEQ_CST;
	default:
		return memorder;
	}
}

CEL_INLINE int cel_pvt_atomic_fail_order(int memorder)
{
	switch (memorder) {
	case CEL_ATOMIC_ACQUIRE:
	case CEL_ATOMIC_ACQ_REL:
		return CEL_ATOMIC_ACQUIRE;
	default:
		return CEL_ATOMIC_RELAXED;
	}
}

/* Mutex Boilerplate */

#if CEL_ATOMIC_USE == CEL_ATOMIC_MUTEX

CEL_STATIC_ASSERT(
	CEL_ATOMIC_MUTEX_STRIPES >= 1 &&
		(CEL_ATOMIC_MUTEX_STRIPES & (CEL_ATOMIC_MUTEX_STRIPES - 1)) == 0,
	"CEL_ATOMIC_MUTEX_STRIPES must be a power of 2", stripes_not_pow2);

#	ifdef CEL_ATOMIC_IMPLEMENT
pthread_once_t cel_pvt_atomic_inited = PTHREAD_ONCE_INIT;
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
	cel_u32 i;
	for (i = 0; i < CEL_ATOMIC_MUTEX_STRIPES; i++) {
		pthread_mutex_init(&cel_pvt_atomic_mutex[i], NULL);
	}
}

CEL_INLINE cel_u32 cel_pvt_atomic_mutex_lock(const void *ptr)
{
	pthread_once(&cel_pvt_atomic_inited, cel_pvt_atomic_mutex_init);
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

/* Template Macros */

#define CEL_ATOMIC_RETURN(rettype, value)                                      \
	CEL_CAT(CEL_ATOMIC_RETURN_, rettype)(value)
#define CEL_ATOMIC_RETURN_int(value) return (value)
#define CEL_ATOMIC_RETURN_void(value)                                          \
	((void)(value));                                                           \
	return

#define CEL_ATOMIC_API(fn, u_or_i, size)                                       \
	CEL_CAT3(CEL_CAT(cel_atomic_, u_or_i), CEL_CAT(size, _), fn)

#define CEL_ATOMIC_WIN_FN(fn, size) CEL_CAT(CEL_ATOMIC_WIN_FN_, size)(fn)
#define CEL_ATOMIC_WIN_FN_8(fn) CEL_CAT(fn, 8)
#define CEL_ATOMIC_WIN_FN_16(fn) CEL_CAT(fn, 16)
#define CEL_ATOMIC_WIN_FN_32(fn) fn
#define CEL_ATOMIC_WIN_FN_64(fn) CEL_CAT(fn, 64)

#define CEL_ATOMIC_SWAP(fn)                                                    \
	return fn(ptr, value, cel_pvt_atomic_memorder(memorder));

#define CEL_ATOMIC_LOAD(fn)                                                    \
	return fn(                                                                 \
		ptr, cel_pvt_atomic_memorder(cel_pvt_atomic_read_order(memorder)));

#define CEL_ATOMIC_STORE(fn)                                                   \
	fn(ptr, value,                                                             \
	   cel_pvt_atomic_memorder(cel_pvt_atomic_write_order(memorder)));

/* Stdatomic Templates */

#if CEL_ATOMIC_USE == CEL_ATOMIC_STD

#	define cel_atomic_init(ptr, value) atomic_init(ptr, value)
#	define CEL_ATOMIC_STD_OP(op)                                              \
		return CEL_CAT3(atomic_fetch_, op, _explicit)(                         \
			ptr, value, cel_pvt_atomic_memorder(memorder));

#	define CEL_ATOMIC_cas(u_or_i, size)                                       \
		atomic_compare_exchange_strong_explicit(                               \
			ptr, &expected, desired, cel_pvt_atomic_memorder(memorder),        \
			cel_pvt_atomic_memorder(cel_pvt_atomic_fail_order(memorder)));     \
		return expected;
#	define CEL_ATOMIC_swap(u_or_i, size)                                      \
		CEL_ATOMIC_SWAP(atomic_exchange_explicit)
#	define CEL_ATOMIC_load(u_or_i, size) CEL_ATOMIC_LOAD(atomic_load_explicit)
#	define CEL_ATOMIC_store(u_or_i, size)                                     \
		CEL_ATOMIC_STORE(atomic_store_explicit)
#	define CEL_ATOMIC_add(u_or_i, size) CEL_ATOMIC_STD_OP(add)
#	define CEL_ATOMIC_sub(u_or_i, size) CEL_ATOMIC_STD_OP(sub)
#	define CEL_ATOMIC_or(u_or_i, size) CEL_ATOMIC_STD_OP(or)
#	define CEL_ATOMIC_and(u_or_i, size) CEL_ATOMIC_STD_OP(and)
#	define CEL_ATOMIC_xor(u_or_i, size) CEL_ATOMIC_STD_OP(xor)

/* GNU C Templates */

#elif CEL_ATOMIC_USE == CEL_ATOMIC_GNUC

#	define CEL_ATOMIC_GNUC_OP(op)                                             \
		return CEL_CAT(__atomic_fetch_, op)(                                   \
			ptr, value, cel_pvt_atomic_memorder(memorder));

#	define CEL_ATOMIC_cas(u_or_i, size)                                       \
		__atomic_compare_exchange_n(                                           \
			ptr, &expected, desired, 0, cel_pvt_atomic_memorder(memorder),     \
			cel_pvt_atomic_memorder(cel_pvt_atomic_fail_order(memorder)));     \
		return expected;
#	define CEL_ATOMIC_swap(u_or_i, size) CEL_ATOMIC_SWAP(__atomic_exchange_n)
#	define CEL_ATOMIC_load(u_or_i, size) CEL_ATOMIC_LOAD(__atomic_load_n)
#	define CEL_ATOMIC_store(u_or_i, size) CEL_ATOMIC_STORE(__atomic_store_n)
#	define CEL_ATOMIC_add(u_or_i, size) CEL_ATOMIC_GNUC_OP(add)
#	define CEL_ATOMIC_sub(u_or_i, size) CEL_ATOMIC_GNUC_OP(sub)
#	define CEL_ATOMIC_or(u_or_i, size) CEL_ATOMIC_GNUC_OP(or)
#	define CEL_ATOMIC_and(u_or_i, size) CEL_ATOMIC_GNUC_OP(and)
#	define CEL_ATOMIC_xor(u_or_i, size) CEL_ATOMIC_GNUC_OP(xor)

/* Sync Templates */

#elif CEL_ATOMIC_USE == CEL_ATOMIC_SYNC

#	define CEL_ATOMIC_SYNC_OP(op)                                             \
		return CEL_CAT(__sync_fetch_and_, op)(ptr, value);

#	define CEL_ATOMIC_SYNC_cas(u_or_i, size)                                  \
		return __sync_val_compare_and_swap(ptr, expected, desired);
#	define CEL_ATOMIC_add(u_or_i, size) CEL_ATOMIC_SYNC_OP(add)
#	define CEL_ATOMIC_sub(u_or_i, size) CEL_ATOMIC_SYNC_OP(sub)
#	define CEL_ATOMIC_or(u_or_i, size) CEL_ATOMIC_SYNC_OP(or)
#	define CEL_ATOMIC_and(u_or_i, size) CEL_ATOMIC_SYNC_OP(and)
#	define CEL_ATOMIC_xor(u_or_i, size) CEL_ATOMIC_SYNC_OP(xor)

/* Windows ARM Templates */

#elif CEL_ATOMIC_USE == CEL_ATOMIC_WIN_ARM32 ||                                \
	CEL_ATOMIC_USE == CEL_ATOMIC_WIN_ARM64

#	define CEL_ATOMIC_WIN_ARM_TEMPLATE(fn, rettype, size, args)               \
		switch (memorder) {                                                    \
		case CEL_ATOMIC_RELAXED:                                               \
			CEL_ATOMIC_RETURN(                                                 \
				rettype,                                                       \
				CEL_APPLY(CEL_CAT(CEL_ATOMIC_WIN_FN(fn, size), _nf), args));   \
		case CEL_ATOMIC_ACQUIRE:                                               \
			CEL_ATOMIC_RETURN(                                                 \
				rettype,                                                       \
				CEL_APPLY(CEL_CAT(CEL_ATOMIC_WIN_FN(fn, size), _acq), args));  \
		case CEL_ATOMIC_RELEASE:                                               \
			CEL_ATOMIC_RETURN(                                                 \
				rettype,                                                       \
				CEL_APPLY(CEL_CAT(CEL_ATOMIC_WIN_FN(fn, size), _rel), args));  \
		default:                                                               \
			CEL_ATOMIC_RETURN(                                                 \
				rettype, CEL_APPLY(CEL_ATOMIC_WIN_FN(fn, size), args));        \
		}
#	define CEL_ATOMIC_WIN_ARM_OP(fn, size)                                    \
		CEL_ATOMIC_WIN_ARM_TEMPLATE(fn, int, size, (ptr, value))

#	define CEL_ATOMIC_cas(u_or_i, size)                                       \
		CEL_ATOMIC_WIN_ARM_TEMPLATE(                                           \
			_InterlockedCompareExchange, int, size, (ptr, desired, expected))
#	define CEL_ATOMIC_swap(u_or_i, size)                                      \
		CEL_ATOMIC_WIN_ARM_OP(_InterlockedExchange, size)
#	define CEL_ATOMIC_add(u_or_i, size)                                       \
		CEL_ATOMIC_WIN_ARM_OP(_InterlockedAdd, size)
#	define CEL_ATOMIC_or(u_or_i, size)                                        \
		CEL_ATOMIC_WIN_ARM_OP(_InterlockedOr, size)
#	define CEL_ATOMIC_and(u_or_i, size)                                       \
		CEL_ATOMIC_WIN_ARM_OP(_InterlockedAnd, size)
#	define CEL_ATOMIC_xor(u_or_i, size)                                       \
		CEL_ATOMIC_WIN_ARM_OP(_InterlockedXor, size)

/* Windows x86-64 Templates */

#elif CEL_ATOMIC_USE == CEL_ATOMIC_WIN_X86 ||                                  \
	CEL_ATOMIC_USE == CEL_ATOMIC_WIN_X64

#	define CEL_ATOMIC_WIN_OP(fn, size)                                        \
		return CEL_ATOMIC_WIN_FN(fn, size)(ptr, value);

#	define CEL_ATOMIC_cas(u_or_i, size)                                       \
		return CEL_ATOMIC_WIN_FN(_InterlockedCompareExchange, size)(           \
			ptr, desired, expected);
#	define CEL_ATOMIC_swap(u_or_i, size)                                      \
		CEL_ATOMIC_WIN_OP(_InterlockedExchange, size)
#	define CEL_ATOMIC_add(u_or_i, size)                                       \
		CEL_ATOMIC_WIN_OP(_InterlockedAdd, size)
#	define CEL_ATOMIC_or(u_or_i, size) CEL_ATOMIC_WIN_OP(_InterlockedOr, size)
#	define CEL_ATOMIC_and(u_or_i, size)                                       \
		CEL_ATOMIC_WIN_OP(_InterlockedAnd, size)
#	define CEL_ATOMIC_xor(u_or_i, size)                                       \
		CEL_ATOMIC_WIN_OP(_InterlockedXor, size)

/* Mutex Templates */

#elif CEL_ATOMIC_USE == CEL_ATOMIC_MUTEX

#	define CEL_ATOMIC_MUTEX_TEMPLATE(expr, rettype, u_or_i, size)             \
		cel_u32 index = cel_pvt_atomic_mutex_lock(ptr);                        \
		CEL_CAT3(cel_, u_or_i, size) retval = *ptr;                            \
		*ptr = (expr);                                                         \
		cel_pvt_atomic_mutex_unlock(index);                                    \
		CEL_ATOMIC_RETURN(rettype, retval);
#	define CEL_ATOMIC_MUTEX_OP(op, u_or_i, size)                              \
		CEL_ATOMIC_MUTEX_TEMPLATE(*ptr op value, int, u_or_i, size)

#	define CEL_ATOMIC_cas(u_or_i, size)                                       \
		CEL_ATOMIC_MUTEX_TEMPLATE(                                             \
			(*ptr == expected) ? desired : *ptr, int, u_or_i, size)
#	define CEL_ATOMIC_swap(u_or_i, size)                                      \
		CEL_ATOMIC_MUTEX_TEMPLATE(value, int, u_or_i, size)
#	define CEL_ATOMIC_load(u_or_i, size)                                      \
		CEL_ATOMIC_MUTEX_TEMPLATE(*ptr, int, u_or_i, size)
#	define CEL_ATOMIC_store(u_or_i, size)                                     \
		CEL_ATOMIC_MUTEX_TEMPLATE(value, void, u_or_i, size)
#	define CEL_ATOMIC_add(u_or_i, size) CEL_ATOMIC_MUTEX_OP(+, u_or_i, size)
#	define CEL_ATOMIC_sub(u_or_i, size) CEL_ATOMIC_MUTEX_OP(-, u_or_i, size)
#	define CEL_ATOMIC_or(u_or_i, size) CEL_ATOMIC_MUTEX_OP(|, u_or_i, size)
#	define CEL_ATOMIC_and(u_or_i, size) CEL_ATOMIC_MUTEX_OP(&, u_or_i, size)
#	define CEL_ATOMIC_xor(u_or_i, size) CEL_ATOMIC_MUTEX_OP(^, u_or_i, size)

#endif

/* Fill-In Templates */

#define CEL_ATOMIC_CAS_LOOP(expr, rettype, u_or_i, size)                       \
	CEL_CAT3(cel_, u_or_i, size) expected, actual = 0;                         \
	do {                                                                       \
		expected = actual;                                                     \
		actual =                                                               \
			CEL_ATOMIC_API(cas, u_or_i, size)(ptr, expected, expr, memorder);  \
	} while (expected != actual);                                              \
	CEL_ATOMIC_RETURN(rettype, actual);

#ifndef CEL_ATOMIC_swap
#	define CEL_ATOMIC_swap(u_or_i, size)                                      \
		CEL_ATOMIC_CAS_LOOP(value, int, u_or_i, size)
#endif

#ifndef CEL_ATOMIC_load
#	define CEL_ATOMIC_load(u_or_i, size)                                      \
		return CEL_ATOMIC_API(or, u_or_i, size)(                               \
			ptr, 0, cel_pvt_atomic_read_order(memorder));
#endif

#ifndef CEL_ATOMIC_store
#	define CEL_ATOMIC_store(u_or_i, size)                                     \
		CEL_ATOMIC_API(swap, u_or_i, size)(                                    \
			ptr, value, cel_pvt_atomic_write_order(memorder));
#endif

#ifndef CEL_ATOMIC_sub
#	define CEL_ATOMIC_sub(u_or_i, size)                                       \
		CEL_ATOMIC_API(add, u_or_i, size)(ptr, -value, memorder)
#endif

#define CEL_ATOMIC_cas64(u_or_i) CEL_ATOMIC_cas(u_or_i, 64)
#if CEL_ATOMIC_USE == CEL_ATOMIC_WIN_ARM32 ||                                  \
	CEL_ATOMIC_USE == CEL_ATOMIC_WIN_X86
#	define CEL_ATOMIC_swap64(u_or_i)                                          \
		CEL_ATOMIC_CAS_LOOP(value, int, u_or_i, 64, memorder)
#	define CEL_ATOMIC_store64(u_or_i)                                         \
		CEL_ATOMIC_API(swap, u_or_i, 64)(                                      \
			ptr, value, cel_pvt_atomic_write_order(memorder));
#	define CEL_ATOMIC_load64(u_or_i)                                          \
		return CEL_ATOMIC_API(or, u_or_i, 64)(                                 \
			ptr, 0, cel_pvt_atomic_read_order(memorder));
#	define CEL_ATOMIC_add64(u_or_i)                                           \
		CEL_ATOMIC_CAS_LOOP(actual + value, int, u_or_i, 64, memorder)
#	define CEL_ATOMIC_sub64(u_or_i)                                           \
		CEL_ATOMIC_CAS_LOOP(actual - value, int, u_or_i, 64, memorder)
#	define CEL_ATOMIC_or64(u_or_i)                                            \
		CEL_ATOMIC_CAS_LOOP(actual | value, int, u_or_i, 64, memorder)
#	define CEL_ATOMIC_and64(u_or_i)                                           \
		CEL_ATOMIC_CAS_LOOP(actual &value, int, u_or_i, 64, memorder)
#	define CEL_ATOMIC_xor64(u_or_i)                                           \
		CEL_ATOMIC_CAS_LOOP(actual ^ value, int, u_or_i, 64, memorder)
#else
#	define CEL_ATOMIC_swap64(u_or_i) CEL_ATOMIC_swap(u_or_i, 64)
#	define CEL_ATOMIC_store64(u_or_i) CEL_ATOMIC_store(u_or_i, 64)
#	define CEL_ATOMIC_load64(u_or_i) CEL_ATOMIC_load(u_or_i, 64)
#	define CEL_ATOMIC_add64(u_or_i) CEL_ATOMIC_add(u_or_i, 64)
#	define CEL_ATOMIC_sub64(u_or_i) CEL_ATOMIC_sub(u_or_i, 64)
#	define CEL_ATOMIC_or64(u_or_i) CEL_ATOMIC_or(u_or_i, 64)
#	define CEL_ATOMIC_and64(u_or_i) CEL_ATOMIC_and(u_or_i, 64)
#	define CEL_ATOMIC_xor64(u_or_i) CEL_ATOMIC_xor(u_or_i, 64)
#endif

/* Template Instantiation */

#define CEL_ATOMIC_RETTYPE(fn) CEL_CAT(CEL_ATOMIC_RETTYPE_, fn)
#define CEL_ATOMIC_RETTYPE_cas int
#define CEL_ATOMIC_RETTYPE_swap int
#define CEL_ATOMIC_RETTYPE_store void
#define CEL_ATOMIC_RETTYPE_load int
#define CEL_ATOMIC_RETTYPE_add int
#define CEL_ATOMIC_RETTYPE_sub int
#define CEL_ATOMIC_RETTYPE_or int
#define CEL_ATOMIC_RETTYPE_and int
#define CEL_ATOMIC_RETTYPE_xor int

#define CEL_ATOMIC_RETSIG(fn, u_or_i, size)                                    \
	CEL_INLINE CEL_CAT(CEL_ATOMIC_RETSIG_, CEL_ATOMIC_RETTYPE(fn))(u_or_i, size)
#define CEL_ATOMIC_RETSIG_int(u_or_i, size) CEL_CAT3(cel_, u_or_i, size)
#define CEL_ATOMIC_RETSIG_void(u_or_i, size) void

#define CEL_ATOMIC_ARGS(fn, u_or_i, size)                                      \
	CEL_CAT(CEL_ATOMIC_ARGS_, fn)(u_or_i, size)
#define CEL_ATOMIC_ARGS_cas(u_or_i, size)                                      \
	CEL_CAT3(cel_atomic_, u_or_i, size) * ptr,                                 \
		CEL_CAT3(cel_, u_or_i, size) expected,                                 \
		CEL_CAT3(cel_, u_or_i, size) desired, int memorder
#define CEL_ATOMIC_ARGS_swap(u_or_i, size)                                     \
	CEL_CAT3(cel_atomic_, u_or_i, size) * ptr,                                 \
		CEL_CAT3(cel_, u_or_i, size) value, int memorder
#define CEL_ATOMIC_ARGS_store CEL_ATOMIC_ARGS_swap
#define CEL_ATOMIC_ARGS_load(u_or_i, size)                                     \
	CEL_CAT3(cel_atomic_, u_or_i, size) * ptr, int memorder
#define CEL_ATOMIC_ARGS_add CEL_ATOMIC_ARGS_swap
#define CEL_ATOMIC_ARGS_sub CEL_ATOMIC_ARGS_swap
#define CEL_ATOMIC_ARGS_or CEL_ATOMIC_ARGS_swap
#define CEL_ATOMIC_ARGS_and CEL_ATOMIC_ARGS_swap
#define CEL_ATOMIC_ARGS_xor CEL_ATOMIC_ARGS_swap

#define CEL_ATOMIC_TEMPLATE(fn, u_or_i, size)                                  \
	CEL_ATOMIC_RETSIG(fn, u_or_i, size)                                        \
	CEL_ATOMIC_API(fn, u_or_i, size)(CEL_ATOMIC_ARGS(fn, u_or_i, size))        \
	{                                                                          \
		CEL_CAT(CEL_ATOMIC_, fn)(u_or_i, size)                                 \
	}

#if CEL_HAS_INT64
#	define CEL_ATOMIC_TEMPLATE64(fn, u_or_i)                                  \
		CEL_ATOMIC_RETSIG(fn, u_or_i, 64)                                      \
		CEL_ATOMIC_API(fn, u_or_i, 64)(CEL_ATOMIC_ARGS(fn, u_or_i, 64))        \
		{                                                                      \
			CEL_CAT3(CEL_ATOMIC_, fn, 64)(u_or_i)                              \
		}
#else
#	define CEL_ATOMIC_TEMPLATE64(fn, u_or_i)
#endif

#define CEL_ATOMIC_INSTANTIATE(fn)                                             \
	CEL_ATOMIC_TEMPLATE(fn, u, 8)                                              \
	CEL_ATOMIC_TEMPLATE(fn, i, 8)                                              \
	CEL_ATOMIC_TEMPLATE(fn, u, 16)                                             \
	CEL_ATOMIC_TEMPLATE(fn, i, 16)                                             \
	CEL_ATOMIC_TEMPLATE(fn, u, 32)                                             \
	CEL_ATOMIC_TEMPLATE(fn, i, 32)                                             \
	CEL_ATOMIC_TEMPLATE64(fn, u)                                               \
	CEL_ATOMIC_TEMPLATE64(fn, i)

CEL_ATOMIC_INSTANTIATE(cas);
CEL_ATOMIC_INSTANTIATE(swap);
CEL_ATOMIC_INSTANTIATE(store);
CEL_ATOMIC_INSTANTIATE(load);
CEL_ATOMIC_INSTANTIATE(add);
CEL_ATOMIC_INSTANTIATE(sub);
CEL_ATOMIC_INSTANTIATE(or);
CEL_ATOMIC_INSTANTIATE(and);
CEL_ATOMIC_INSTANTIATE(xor);

/* Completing Integer Functions */

#ifndef cel_atomic_init
#	if CEL_HAS_INT64
#		define cel_atomic_init(ptr, value)                                    \
			do {                                                               \
				switch (sizeof(*ptr)) {                                        \
				case 1:                                                        \
					cel_atomic_u8_store(                                       \
						ptr, (cel_u8)value, CEL_ATOMIC_RELAXED);               \
					break;                                                     \
				case 2:                                                        \
					cel_atomic_u16_store(                                      \
						ptr, (cel_u16)value, CEL_ATOMIC_RELAXED);              \
					break;                                                     \
				case 4:                                                        \
					cel_atomic_u32_store(                                      \
						ptr, (cel_u32)value, CEL_ATOMIC_RELAXED);              \
					break;                                                     \
				case 8:                                                        \
					cel_atomic_u64_store(                                      \
						ptr, (cel_u64)value, CEL_ATOMIC_RELAXED);              \
					break;                                                     \
				}                                                              \
			} while (0)
#	else
#		define cel_atomic_init(ptr, value)                                    \
			do {                                                               \
				switch (sizeof(*ptr)) {                                        \
				case 1:                                                        \
					cel_atomic_u8_store(                                       \
						ptr, (cel_u8)value, CEL_ATOMIC_RELAXED);               \
					break;                                                     \
				case 2:                                                        \
					cel_atomic_u16_store(                                      \
						ptr, (cel_u16)value, CEL_ATOMIC_RELAXED);              \
					break;                                                     \
				case 4:                                                        \
					cel_atomic_u32_store(                                      \
						ptr, (cel_u32)value, CEL_ATOMIC_RELAXED);              \
					break;                                                     \
				}                                                              \
			} while (0)
#	endif
#endif

#if CEL_UPTR_SIZE == 4
#	define CEL_ATOMIC_UPTR_BITS 32
#elif CEL_UPTR_SIZE == 8
#	define CEL_ATOMIC_UPTR_BITS 64
#endif

#define cel_atomic_uptr_cas CEL_ATOMIC_API(cas, u, CEL_ATOMIC_UPTR_BITS)
#define cel_atomic_uptr_swap CEL_ATOMIC_API(swap, u, CEL_ATOMIC_UPTR_BITS)
#define cel_atomic_uptr_store CEL_ATOMIC_API(store, u, CEL_ATOMIC_UPTR_BITS)
#define cel_atomic_uptr_load CEL_ATOMIC_API(load, u, CEL_ATOMIC_UPTR_BITS)
#define cel_atomic_uptr_add CEL_ATOMIC_API(add, u, CEL_ATOMIC_UPTR_BITS)
#define cel_atomic_uptr_sub CEL_ATOMIC_API(sub, u, CEL_ATOMIC_UPTR_BITS)
#define cel_atomic_uptr_or CEL_ATOMIC_API(or, u, CEL_ATOMIC_UPTR_BITS)
#define cel_atomic_uptr_and CEL_ATOMIC_API(and, u, CEL_ATOMIC_UPTR_BITS)
#define cel_atomic_uptr_xor CEL_ATOMIC_API(xor, u, CEL_ATOMIC_UPTR_BITS)

#if CEL_SIZE_BYTES == 8
#	define CEL_ATOMIC_SIZE_BITS 64
#elif CEL_SIZE_BYTES == 4
#	define CEL_ATOMIC_SIZE_BITS 32
#elif CEL_SIZE_BYTES == 2
#	define CEL_ATOMIC_SIZE_BITS 16
#elif CEL_SIZE_BYTES == 1
#	define CEL_ATOMIC_SIZE_BITS 8
#endif

#define cel_atomic_size_cas CEL_ATOMIC_API(cas, u, CEL_ATOMIC_SIZE_BITS)
#define cel_atomic_size_swap CEL_ATOMIC_API(swap, u, CEL_ATOMIC_SIZE_BITS)
#define cel_atomic_size_store CEL_ATOMIC_API(store, u, CEL_ATOMIC_SIZE_BITS)
#define cel_atomic_size_load CEL_ATOMIC_API(load, u, CEL_ATOMIC_SIZE_BITS)
#define cel_atomic_size_add CEL_ATOMIC_API(add, u, CEL_ATOMIC_SIZE_BITS)
#define cel_atomic_size_sub CEL_ATOMIC_API(sub, u, CEL_ATOMIC_SIZE_BITS)
#define cel_atomic_size_or CEL_ATOMIC_API(or, u, CEL_ATOMIC_SIZE_BITS)
#define cel_atomic_size_and CEL_ATOMIC_API(and, u, CEL_ATOMIC_SIZE_BITS)
#define cel_atomic_size_xor CEL_ATOMIC_API(xor, u, CEL_ATOMIC_SIZE_BITS)

/* Atomic Flags */

#if CEL_ATOMIC_USE == CEL_ATOMIC_STD

typedef atomic_flag cel_atomic_flag;
#	define CEL_ATOMIC_FLAG_INIT ATOMIC_FLAG_INIT

CEL_INLINE cel_bool cel_atomic_flag_set(cel_atomic_flag *ptr, int memorder)
{
	return atomic_flag_test_and_set_explicit(
		ptr, cel_pvt_atomic_memorder(memorder));
}

CEL_INLINE void cel_atomic_flag_clear(cel_atomic_flag *ptr, int memorder)
{
	atomic_flag_clear_explicit(
		ptr, cel_pvt_atomic_memorder(cel_pvt_atomic_write_order(memorder)));
}

#elif CEL_ATOMIC_USE == CEL_ATOMIC_SYNC

typedef cel_atomic_u8 cel_atomic_flag;
#	define CEL_ATOMIC_FLAG_INIT ((cel_atomic_flag)0)

CEL_INLINE cel_bool cel_atomic_flag_set(cel_atomic_flag *ptr, int memorder)
{
	switch (memorder) {
	case CEL_ATOMIC_RELAXED:
	case CEL_ATOMIC_ACQUIRE:
		return __sync_lock_test_and_set(ptr, 1);
	default:
		return __sync_fetch_and_or(ptr, 1);
	}
}

CEL_INLINE void cel_atomic_flag_clear(cel_atomic_flag *ptr, int memorder)
{
	switch (memorder) {
	case CEL_ATOMIC_RELAXED:
	case CEL_ATOMIC_RELEASE:
		__sync_lock_release(ptr);
		break;
	default:
		__sync_fetch_and_and(ptr, 0);
		break;
	}
}

#else

typedef cel_atomic_u8 cel_atomic_flag;
#	define CEL_ATOMIC_FLAG_INIT ((cel_atomic_flag)0)

CEL_INLINE cel_bool cel_atomic_flag_set(cel_atomic_flag *ptr, int memorder)
{
	return cel_atomic_u8_swap(ptr, 1, memorder);
}

CEL_INLINE void cel_atomic_flag_clear(cel_atomic_flag *ptr, int memorder)
{
	cel_atomic_u8_store(ptr, 0, memorder);
}

#endif

#endif
