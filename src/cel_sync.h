#ifndef CEL_SYNC_H
#define CEL_SYNC_H

#include "cel_core.h"

/* Configuration Knobs */

#ifndef CEL_SYNC_SPIN_INIT
#	define CEL_SYNC_SPIN_INIT 4u
#endif

#ifndef CEL_SYNC_SPIN_CAP
#	define CEL_SYNC_SPIN CAP 1u << 12
#endif

/* Determine Header Availability */

#if defined(_WIN32) || defined(_WIN64)
#	include <windows.h>
#	define CEL_SYNC_HAS_WIN 1
#else
#	define CEL_SYNC_HAS_WIN 0
#endif

#if defined(__unix__) || defined(__unix) || defined(unix) ||                   \
	defined(__APPLE__) || defined(__MACH__) || defined(__linux__) ||           \
	defined(__ANDROID__) || defined(__CYGWIN__) || defined(__EMSCRIPTEN__) ||  \
	defined(__wasi__)
#	include <unistd.h>
#	if defined(_POSIX_THREADS)
#		include <pthread.h>
#		define CEL_SYNC_HAS_PTHREAD 1
#	else
#		define CEL_SYNC_HAS_PTHREAD 0
#	endif
#else
#	define CEL_SYNC_HAS_PTHREAD 0
#endif

#if CEL_STDC >= CEL_STDC11 && !defined(__STDC_NO_THREADS__)
#	include <threads.h>
#	define CEL_SYNC_HAS_C11 1
#else
#	define CEL_SYNC_HAS_C11 0
#endif

#include "cel_atomic.h"
#if CEL_ATOMIC_USE != CEL_ATOMIC_MUTEX
#	define CEL_SYNC_HAS_ATOMICS 1
#else
#	define CEL_SYNC_HAS_ATOMICS 0
#else
#	define CEL_SYNC_HAS_ATOMICS 0
#endif

CEL_STATIC_ASSERT(
	CEL_SYNC_HAS_PTHREAD || CEL_SYNC_HAS_C11 || CEL_SYNC_HAS_WIN ||
		CEL_SYNC_HAS_ATOMICS,
	"Unable to find any synchronization primitives", no_sync_primitives);

/* Status Codes */

enum {
	CEL_SYNC_E_AGAIN = -1,
	CEL_SYNC_E_NOMEM = -2,
	CEL_SYNC_E_PERM = -3,
	CEL_SYNC_E_INVAL = -4,
	CEL_SYNC_E_TIMEDOUT = -5,
	CEL_SYNC_E_UNKNOWN = -6,

	CEL_SYNC_SUCCESS = 0,
	CEL_SYNC_BUSY
};

#if CEL_SYNC_HAS_PTHREAD
CEL_INLINE int cel_pvt_sync_pthread_status(int status)
{
	switch (status) {
	case 0:
		return CEL_SYNC_SUCCESS;
	case EAGAIN:
		return CEL_SYNC_E_AGAIN;
	case ENOMEM:
		return CEL_SYNC_E_NOMEM;
	case EPERM:
		return CEL_SYNC_E_PERM;
	case EINVAL:
		return CEL_SYNC_E_INVAL;
	case ETIMEDOUT:
		return CEL_SYNC_E_TIMEDOUT;
	case EBUSY:
		return CEL_SYNC_BUSY;
	default:
		return CEL_SYNC_E_UNKNOWN;
	}
}
#endif
#if CEL_SYNC_HAS_C11
CEL_INLINE int cel_pvt_sync_c11_status(int status)
{
	switch (status) {
	case thrd_success:
		return CEL_SYNC_SUCCESS;
	case thrd_nomem:
		return CEL_SYNC_E_NOMEM;
	case thrd_timedout:
		return CEL_SYNC_E_TIMEDOUT;
	case thrd_busy:
		return CEL_SYNC_BUSY;
	default:
		return CEL_SYNC_E_UNKNOWN;
	}
}
#endif

/* Helper Macros */

#define CEL_SYNC_ASSERT(call)                                                  \
	do {                                                                       \
		int retval = (call);                                                   \
		if (retval) {                                                          \
			return retval;                                                     \
		}                                                                      \
	} while (0)

#define CEL_SYNC_ASSERT_CLEANUP(call, cleanup)                                 \
	do {                                                                       \
		int retval = (call);                                                   \
		if (retval) {                                                          \
			cleanup;                                                           \
			return retval;                                                     \
		}                                                                      \
	} while (0)

/* Mutexes */

#define CEL_SYNC_WIN 4
#define CEL_SYNC_PTHREAD 3
#define CEL_SYNC_C11 2
#define CEL_SYNC_FALLBACK 1

#if CEL_SYNC_HAS_WIN
#	define CEL_SYNC_MUTEX_USE CEL_SYNC_WIN
typedef CRITICAL_SECTION cel_sync_mutex;
#elif CEL_SYNC_HAS_PTHREAD
#	define CEL_SYNC_MUTEX_USE CEL_SYNC_PTHREAD
typedef pthread_mutex_t cel_sync_mutex;
#elif CEL_SYNC_HAS_C11
#	define CEL_SYNC_MUTEX_USE CEL_SYNC_C11
typedef mtx_t cel_sync_mutex;
#else
#	define CEL_SYNC_MUTEX_USE CEL_SYNC_FALLBACK
typedef cel_atomic_flag cel_sync_mutex;
#endif

#ifdef CEL_SYNC_IMPLEMENT
int cel_sync_mutex_init(cel_sync_mutex *mutex);
int cel_sync_mutex_destroy(cel_sync_mutex *mutex);
int cel_sync_mutex_lock(cel_sync_mutex *mutex);
int cel_sync_mutex_trylock(cel_sync_mutex *mutex);
int cel_sync_mutex_unlock(cel_sync_mutex *mutex);
#elif CEL_SYNC_MUTEX_USE == CEL_SYNC_WIN

int cel_sync_mutex_init(cel_sync_mutex *mutex)
{
	InitializeCriticalSection(mutex);
	return CEL_SYNC_SUCCESS;
}

int cel_sync_mutex_destroy(cel_sync_mutex *mutex)
{
	DeleteCriticalSection(mutex);
	return CEL_SYNC_SUCCESS;
}

int cel_sync_mutex_lock(cel_sync_mutex *mutex)
{
	EnterCriticalSection(mutex);
	return CEL_SYNC_SUCCESS;
}

int cel_sync_mutex_trylock(cel_sync_mutex *mutex)
{
	TryEnterCriticalSection(mutex);
	return CEL_SYNC_SUCCESS;
}

int cel_sync_mutex_unlock(cel_sync_mutex *mutex)
{
	LeaveCriticalSection(mutex);
	return CEL_SYNC_SUCCESS;
}

#elif CEL_SYNC_MUTEX_USE == CEL_SYNC_PTHREAD

int cel_sync_mutex_init(cel_sync_mutex *mutex)
{
	return cel_pvt_sync_pthread_status(pthread_mutex_init(mutex, NULL));
}

int cel_sync_mutex_destroy(cel_sync_mutex *mutex)
{
	return cel_pvt_sync_pthread_status(pthread_mutex_destroy(mutex));
}

int cel_sync_mutex_lock(cel_sync_mutex *mutex)
{
	return cel_pvt_sync_pthread_status(pthread_mutex_lock(mutex));
}

int cel_sync_mutex_trylock(cel_sync_mutex *mutex)
{
	return cel_pvt_sync_pthread_status(pthread_mutex_trylock(mutex));
}

int cel_sync_mutex_unlock(cel_sync_mutex *mutex)
{
	return cel_pvt_sync_pthread_status(pthread_mutex_unlock(mutex));
}

#elif CEL_SYNC_MUTEX_USE == CEL_SYNC_C11

int cel_sync_mutex_init(cel_sync_mutex *mutex)
{
	return cel_pvt_sync_c11_status(mtx_init(mutex, mtx_plain));
}

int cel_sync_mutex_destroy(cel_sync_mutex *mutex)
{
	mtx_destroy(mutex);
	return CEL_SYNC_SUCCESS;
}

int cel_sync_mutex_lock(cel_sync_mutex *mutex)
{
	return cel_pvt_sync_c11_status(mtx_lock(mutex));
}

int cel_sync_mutex_trylock(cel_sync_mutex *mutex)
{
	return cel_pvt_sync_c11_status(mtx_trylock(mutex));
}

int cel_sync_mutex_unlock(cel_sync_mutex *mutex)
{
	return cel_pvt_sync_c11_status(mtx_unlock(mutex));
}

#else

int cel_sync_mutex_init(cel_sync_mutex *mutex)
{
	cel_atomic_init(mutex, 0);
	return CEL_SYNC_SUCCESS;
}

int cel_sync_mutex_destroy(cel_sync_mutex *mutex)
{
	return CEL_SYNC_SUCCESS;
}

int cel_sync_mutex_lock(cel_sync_mutex *mutex)
{
	CEL_POLL(
		CEL_SYNC_SPIN_INIT, CEL_SYNC_SPIN_CAP,
		cel_atomic_flag_set(mutex, CEL_ATOMIC_ACQUIRE));
	return CEL_SYNC_SUCCESS;
}

int cel_sync_mutex_trylock(cel_sync_mutex *mutex)
{
	if (cel_atomic_flag_set(mutex, CEL_ATOMIC_ACQUIRE)) {
		return CEL_SYNC_BUSY;
	}
	return CEL_SYNC_SUCCESS;
}

int cel_sync_mutex_unlock(cel_sync_mutex *mutex)
{
	cel_atomic_flag_clear(mutex, CEL_ATOMIC_RELEASE);
	return CEL_SYNC_SUCCESS;
}

#endif

/* Spinlocks */

#if CEL_SYNC_HAS_ATOMICS
typedef cel_atomic_flag cel_sync_spinlock;
#else
typedef cel_sync_mutex cel_sync_spinlock;
#endif

#ifndef CEL_SYNC_IMPLEMENT
int cel_sync_spinlock_init(cel_sync_spinlock *spinlock);
int cel_sync_spinlock_destroy(cel_sync_spinlock *spinlock);
int cel_sync_spinlock_lock(cel_sync_spinlock *spinlock);
int cel_sync_spinlock_trylock(cel_sync_spinlock *spinlock);
int cel_sync_spinlock_unlock(cel_sync_spinlock *spinlock);
#elif CEL_SYNC_HAS_ATOMICS

int cel_sync_spinlock_init(cel_sync_spinlock *spinlock)
{
	cel_atomic_init(spinlock, 0);
	return CEL_SYNC_SUCCESS;
}

int cel_sync_spinlock_destroy(cel_sync_spinlock *spinlock)
{
	return CEL_SYNC_SUCCESS;
}

int cel_sync_spinlock_lock(cel_sync_spinlock *spinlock)
{
	CEL_POLL(
		CEL_SYNC_SPIN_INIT, CEL_SYNC_SPIN_CAP,
		cel_atomic_flag_set(spinlock, CEL_ATOMIC_ACQUIRE));
	return CEL_SYNC_SUCCESS;
}

int cel_sync_spinlock_trylock(cel_sync_spinlock *spinlock)
{
	if (cel_atomic_flag_set(spinlock, CEL_ATOMIC_ACQUIRE)) {
		return CEL_SYNC_BUSY;
	}
	return CEL_SYNC_SUCCESS;
}

int cel_sync_spinlock_unlock(cel_sync_spinlock *spinlock)
{
	cel_atomic_flag_clear(spinlock, CEL_ATOMIC_RELEASE);
	return CEL_SYNC_SUCCESS;
}

#else

int cel_sync_spinlock_init(cel_sync_spinlock *spinlock)
{
	return cel_sync_mutex_init(spinlock);
}

int cel_sync_spinlock_destroy(cel_sync_spinlock *spinlock)
{
	return cel_sync_mutex_destroy(spinlock);
}

int cel_sync_spinlock_lock(cel_sync_spinlock *spinlock)
{
	return cel_sync_mutex_lock(spinlock);
}

int cel_sync_spinlock_trylock(cel_sync_spinlock *spinlock)
{
	return cel_sync_mutex_trylock(spinlock);
}

int cel_sync_spinlock_unlock(cel_sync_spinlock *spinlock)
{
	return cel_sync_mutex_unlock(spinlock);
}

#endif

/* Condition Variables */

#if CEL_SYNC_MUTEX_USE == CEL_SYNC_WIN
typedef CONDITION_VARIABLE cel_sync_cond;
#elif CEL_SYNC_MUTEX_USE == CEL_SYNC_PTHREAD
typedef pthread_cond_t cel_sync_cond;
#elif CEL_SYNC_MUTEX_USE == CEL_SYNC_C11
typedef cnd_t cel_sync_cond;
#else
typedef struct {
	cel_u16 wait_id;
	cel_u16 signal_id;
	cel_sync_mutex mutex;
} cel_sync_cond;
#endif

#if CEL_SYNC_IMPLEMENT
int cel_sync_cond_init(cel_sync_cond *cond);
int cel_sync_cond_destroy(cel_sync_cond *cond);
int cel_sync_cond_wait(cel_sync_cond *cond, cel_sync_mutex *mutex);
int cel_sync_cond_signal(cel_sync_cond *cond);
int cel_sync_cond_broadcast(cel_sync_cond *cond);
#elif CEL_SYNC_MUTEX_USE == CEL_SYNC_WIN

int cel_sync_cond_init(cel_sync_cond *cond)
{
	InitializeConditionVariable(cond);
	return CEL_SYNC_SUCCESS;
}

int cel_sync_cond_destroy(cel_sync_cond *cond)
{
	return CEL_SYNC_SUCCESS;
}

int cel_sync_cond_wait(cel_sync_cond *cond, cel_sync_mutex *mutex)
{
	if (SleepConditionVariableCS(cond, mutex, INFINITE)) {
		return CEL_SYNC_SUCCESS;
	}
	return cel_pvt_sync_win_status(GetLastError());
}

int cel_sync_cond_signal(cel_sync_cond *cond)
{
	WakeConditionVariable(cond);
	return CEL_SYNC_SUCCESS;
}

int cel_sync_cond_broadcast(cel_sync_cond *cond)
{
	WakeAllConditionVariable(cond);
	return CEL_SYNC_SUCCESS;
}

#elif CEL_SYNC_MUTEX_USE == CEL_SYNC_PTHREAD

int cel_sync_cond_init(cel_sync_cond *cond)
{
	return cel_pvt_sync_pthread_status(pthread_cond_init(cond, NULL));
}

int cel_sync_cond_destroy(cel_sync_cond *cond)
{
	return cel_pvt_sync_pthread_status(pthread_cond_destroy(cond));
}

int cel_sync_cond_wait(cel_sync_cond *cond, cel_sync_mutex *mutex)
{
	return cel_pvt_sync_pthread_status(pthread_cond_wait(cond, mutex));
}

int cel_sync_cond_signal(cel_sync_cond *cond)
{
	return cel_pvt_sync_pthread_status(pthread_cond_signal(cond));
}

int cel_sync_cond_broadcast(cel_sync_cond *cond)
{
	return cel_pvt_sync_pthread_status(pthread_cond_signal(cond));
}

#elif CEL_SYNC_MUTEX_USE == CEL_SYNC_C11

int cel_sync_cond_init(cel_sync_cond *cond)
{
	return cel_pvt_sync_c11_status(cnd_init(cond));
}

int cel_sync_cond_destroy(cel_sync_cond *cond)
{
	cnd_destroy(cond);
	return CEL_SYNC_SUCCESS;
}

int cel_sync_cond_wait(cel_sync_cond *cond, cel_sync_mutex *mutex)
{
	return cel_pvt_sync_c11_status(cnd_wait(cond, mutex));
}

int cel_sync_cond_signal(cel_sync_cond *cond)
{
	return cel_pvt_sync_c11_status(cnd_signal(cond));
}

int cel_sync_cond_broadcast(cel_sync_cond *cond)
{
	return cel_pvt_sync_c11_status(cnd_signal(cond));
}

#else

int cel_sync_cond_init(cel_sync_cond *cond)
{
	cond->wait_id = 0;
	cond->signal_id = 0;
	return cel_sync_mutex_init(&cond->mutex);
}

int cel_sync_cond_destroy(cel_sync_cond *cond)
{
	return cel_sync_mutex_destroy(&cond->mutex);
}

CEL_INLINE int cel_pvt_sync_cond_check(cel_sync_cond *cond, cel_u16 wait_id)
{
	CEL_SYNC_ASSERT(cel_sync_mutex_lock(&cond->mutex));

	if (wait_id - cond->signal_id >= cond->wait_id - cond->signal_id) {
		return cel_sync_mutex_unlock(&cond->mutex);
	}

	CEL_SYNC_ASSERT(cel_sync_mutex_unlock(&cond->mutex));
	return CEL_SYNC_BUSY;
}

int cel_sync_cond_wait(cel_sync_cond *cond, cel_sync_mutex *mutex)
{
	CEL_SYNC_ASSERT(cel_sync_mutex_lock(&cond->mutex));
	CEL_SYNC_ASSERT_CLEANUP(
		cel_sync_mutex_unlock(mutex), cel_sync_mutex_unlock(&cond->mutex));

	cel_u16 wait_id = cond->wait_id++;
	CEL_SYNC_ASSERT(cel_sync_mutex_unlock(&cond->mutex));

	CEL_POLL(
		CEL_SPIN_INIT, CEL_SPIN_CAP, cel_pvt_sync_cond_check(cond, wait_id));

	CEL_SYNC_ASSERT(cel_sync_mutex_lock(mutex));
	return CEL_SYNC_SUCCESS;
}

int cel_sync_cond_signal(cel_sync_cond *cond)
{
	CEL_SYNC_ASSERT(cel_sync_mutex_lock(&cond->mutex));

	if (cond->signal_id == cond->wait_id) {
		CEL_SYNC_ASSERT(cel_sync_mutex_unlock(&cond->mutex));
		return CEL_SYNC_SUCCESS;
	}

	cond->signal_id++;

	CEL_SYNC_ASSERT(cel_sync_mutex_unlock(&cond->mutex));
	return CEL_SYNC_SUCCESS;
}

int cel_sync_cond_broadcast(cel_sync_cond *cond)
{
	CEL_SYNC_ASSERT(cel_sync_mutex_lock(&cond->mutex));

	cond->signal_id = cond->wait_id;

	CEL_SYNC_ASSERT(cel_sync_mutex_unlock(&cond->mutex));
	return CEL_SYNC_SUCCESS;
}

#endif

/* RW-Locks */

#if CEL_SYNC_HAS_WIN
#	define CEL_SYNC_RWLOCK_USE CEL_SYNC_WIN
typedef SRWLOCK cel_sync_rwlock;
#elif defined(PTHREAD_RWLOCK_INITIALIZER)
#	define CEL_SYNC_RWLOCK_USE CEL_SYNC_PTHREAD
typedef pthread_rwlock_t cel_sync_rwlock;
#else
#	define CEL_SYNC_RWLOCK_USE CEL_SYNC_FALLBACK
typedef struct {
	cel_u16 readers_waiting;
	cel_u16 writers_waiting;
	cel_sync_mutex mutex;
	cel_sync_cond cond;
} cel_sync_rwlock;
#endif

/* Semaphores */

#if CEL_SYNC_HAS_WIN
#	define CEL_SYNC_SEM_USE CEL_SYNC_WIN
typedef HANDLE *cel_sync_sem;
#elif defined(_POSIX_SEMAPHORES)
#	define CEL_SYNC_SEM_USE CEL_SYNC_PTHREAD
#	include <sem.h>
typedef sem_t cel_sync_sem;
#else
#	define CEL_SYNC_SEM_USE CEL_SYNC_FALLBACK
typedef struct {
	cel_u16 slots;
	cel_sync_mutex mutex;
	cel_sync_cond cond;
} cel_sync_sem;
#endif

#ifndef CEL_SYNC_IMPLEMENT
int cel_sync_sem_init(cel_sync_sem *sem, cel_u16 count);
int cel_sync_sem_destroy(cel_sync_sem *sem);
int cel_sync_sem_wait(cel_sync_sem *sem);
int cel_sync_sem_trywait(cel_sync_sem *sem);
int cel_sync_sem_post(cel_sync_sem *sem);
#elif CEL_SYNC_SEM_USE == CEL_SYNC_WIN

int cel_sync_sem_init(cel_sync_sem *sem, cel_u16 count)
{
	*sem = CreateSemaphoreA(NULL, count, count, NULL);
	if (*sem) {
		return CEL_SYNC_SUCCESS;
	}
	return cel_pvt_sync_win_status(GetLastError());
}

int cel_sync_sem_destroy(cel_sync_sem *sem)
{
	if (CloseHandle(*sem)) {
		return CEL_SYNC_SUCCESS;
	}
	return cel_pvt_sync_win_status(GetLastError());
}

int cel_sync_sem_wait(cel_sync_sem *sem)
{
	if (WaitForSingleObject(*sem, INFINITE) != WAIT_FAILED) {
		return CEL_SYNC_SUCCESS;
	}
	return cel_pvt_sync_win_status(GetLastError());
}

int cel_sync_sem_trywait(cel_sync_sem *sem)
{
	switch (WaitForSingleObject(*sem, 1)) {
	case WAIT_TIMEOUT:
		return CEL_SYNC_BUSY;
	case WAIT_FAILED:
		return cel_pvt_sync_win_status(GetLastError());
	default:
		return CEL_SYNC_SUCCESS;
	}
}

int cel_sync_sem_post(cel_sync_sem *sem)
{
	if (ReleaseSemaphore(*sem, 1, NULL)) {
		return CEL_SYNC_SUCCESS;
	}
	return cel_pvt_sync_win_status(GetLastError());
}

#elif CEL_SYNC_SEM_USE == CEL_SYNC_PTHREAD

int cel_sync_sem_init(cel_sync_sem *sem, cel_u16 count)
{
	return cel_pvt_sync_pthread_status(sem_init(sem, 0, count));
}

int cel_sync_sem_destroy(cel_sync_sem *sem)
{
	return cel_pvt_sync_pthread_status(sem_destroy(sem));
}

int cel_sync_sem_wait(cel_sync_sem *sem)
{
	return cel_pvt_sync_pthread_status(sem_wait(sem));
}

int cel_sync_sem_trywait(cel_sync_sem *sem)
{
	return cel_pvt_sync_pthread_status(sem_trywait(sem));
}

int cel_sync_sem_post(cel_sync_sem *sem)
{
	return cel_pvt_sync_pthread_status(sem_post(sem));
}
}

#else

int cel_sync_sem_init(cel_sync_sem *sem, cel_u16 count)
{
	sem->slots = count;
	CEL_SYNC_ASSERT(cel_sync_mutex_init(&sem->mutex));
	CEL_SYNC_ASSERT_CLEANUP(
		cel_sync_cond_init(&sem->cond), cel_sync_mutex_destroy(&sem->mutex));
	return CEL_SYNC_SUCCESS;
}

int cel_sync_sem_destroy(cel_sync_sem *sem)
{
	CEL_SYNC_ASSERT_CLEANUP(
		cel_sync_cond_destroy(&sem->cond), cel_sync_mutex_destroy(&sem->mutex));
	CEL_SYNC_ASSERT(cel_sync_mutex_destroy(&sem->mutex));
	return CEL_SYNC_SUCCESS;
}

int cel_sync_sem_wait(cel_sync_sem *sem)
{
	CEL_SYNC_ASSERT(cel_sync_mutex_lock(&sem->mutex));

	while (sem->slots == 0) {
		CEL_SYNC_ASSERT_CLEANUP(
			cel_sync_cond_wait(&sem->cond, &sem->mutex),
			cel_sync_mutex_unlock(&sem->mutex));
	}
	sem->slots--;

	CEL_SYNC_ASSERT(cel_sync_mutex_unlock(&sem->mutex));
	return CEL_SYNC_SUCCESS;
}

int cel_sync_sem_trywait(cel_sync_sem *sem)
{
	CEL_SYNC_ASSERT(cel_sync_mutex_lock(&sem->mutex));
	if (sem->slots > 0) {
		sem->slots--;
		CEL_SYNC_ASSERT(cel_sync_mutex_unlock(&sem->mutex));
		return CEL_SYNC_SUCCESS;
	}
	CEL_SYNC_ASSERT(cel_sync_mutex_unlock(&sem->mutex));
	return CEL_SYNC_BUSY;
}

int cel_sync_sem_post(cel_sync_sem *sem)
{
	CEL_SYNC_ASSERT(cel_sync_mutex_lock(&sem->mutex));
	if (0 == sem->slots++) {
		CEL_SYNC_ASSERT_CLEANUP(
			cel_sync_cond_signal(&sem->cond),
			cel_sync_mutex_unlock(&sem->mutex));
	}
	CEL_SYNC_ASSERT(cel_sync_mutex_unlock(&sem->mutex));
	return CEL_SYNC_SUCCESS;
}

#endif

#endif
