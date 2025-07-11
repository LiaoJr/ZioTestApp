/* Copyright Joyent, Inc. and other Node contributors. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "uv.h"
#include "internal.h"

#include <pthread.h>
#include <assert.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/resource.h>  /* getrlimit() */
#include <unistd.h>  /* getpagesize() */

#include <limits.h>

#ifdef __MVS__
#include <sys/ipc.h>
#include <sys/sem.h>
#endif

#undef NANOSEC
#define NANOSEC ((uint64_t) 1e9)


#if defined(UV__PTHREAD_BARRIER_FALLBACK)
/* TODO: support barrier_attr */
int pthread_barrier_init(pthread_barrier_t* barrier,
                         const void* barrier_attr,
                         unsigned count) {
  int rc;
  _uv_barrier* b;

  if (barrier == NULL || count == 0)
    return EINVAL;

  if (barrier_attr != NULL)
    return ENOTSUP;

  b = uv__malloc(sizeof(*b));
  if (b == NULL)
    return ENOMEM;

  b->in = 0;
  b->out = 0;
  b->threshold = count;

  if ((rc = pthread_mutex_init(&b->mutex, NULL)) != 0)
    goto error2;
  if ((rc = pthread_cond_init(&b->cond, NULL)) != 0)
    goto error;

  barrier->b = b;
  return 0;

error:
  pthread_mutex_destroy(&b->mutex);
error2:
  uv__free(b);
  return rc;
}

int pthread_barrier_wait(pthread_barrier_t* barrier) {
  int rc;
  _uv_barrier* b;

  if (barrier == NULL || barrier->b == NULL)
    return EINVAL;

  b = barrier->b;
  /* Lock the mutex*/
  if ((rc = pthread_mutex_lock(&b->mutex)) != 0)
    return rc;

  /* Increment the count. If this is the first thread to reach the threshold,
     wake up waiters, unlock the mutex, then return
     PTHREAD_BARRIER_SERIAL_THREAD. */
  if (++b->in == b->threshold) {
    b->in = 0;
    b->out = b->threshold - 1;
    rc = pthread_cond_signal(&b->cond);
    assert(rc == 0);

    pthread_mutex_unlock(&b->mutex);
    return PTHREAD_BARRIER_SERIAL_THREAD;
  }
  /* Otherwise, wait for other threads until in is set to 0,
     then return 0 to indicate this is not the first thread. */
  do {
    if ((rc = pthread_cond_wait(&b->cond, &b->mutex)) != 0)
      break;
  } while (b->in != 0);

  /* mark thread exit */
  b->out--;
  pthread_cond_signal(&b->cond);
  pthread_mutex_unlock(&b->mutex);
  return rc;
}

int pthread_barrier_destroy(pthread_barrier_t* barrier) {
  int rc;
  _uv_barrier* b;

  if (barrier == NULL || barrier->b == NULL)
    return EINVAL;

  b = barrier->b;

  if ((rc = pthread_mutex_lock(&b->mutex)) != 0)
    return rc;

  if (b->in > 0 || b->out > 0)
    rc = EBUSY;

  pthread_mutex_unlock(&b->mutex);

  if (rc)
    return rc;

  pthread_cond_destroy(&b->cond);
  pthread_mutex_destroy(&b->mutex);
  uv__free(barrier->b);
  barrier->b = NULL;
  return 0;
}
#endif


/* On MacOS, threads other than the main thread are created with a reduced
 * stack size by default.  Adjust to RLIMIT_STACK aligned to the page size.
 *
 * On Linux, threads created by musl have a much smaller stack than threads
 * created by glibc (80 vs. 2048 or 4096 kB.)  Follow glibc for consistency.
 */
static size_t thread_stack_size(void) {
#if defined(__APPLE__) || defined(__linux__)
  struct rlimit lim;

  if (getrlimit(RLIMIT_STACK, &lim))
    abort();

  if (lim.rlim_cur != RLIM_INFINITY) {
    /* pthread_attr_setstacksize() expects page-aligned values. */
    lim.rlim_cur -= lim.rlim_cur % (rlim_t) getpagesize();
    if (lim.rlim_cur >= PTHREAD_STACK_MIN)
      return lim.rlim_cur;
  }
#endif

#if !defined(__linux__)
  return 0;
#elif defined(__PPC__) || defined(__ppc__) || defined(__powerpc__)
  return 4 << 20;  /* glibc default. */
#else
  return 2 << 20;  /* glibc default. */
#endif
}


int uv_thread_create(uv_thread_t *tid, void (*entry)(void *arg), void *arg) {
  int err;
  size_t stack_size;
  pthread_attr_t* attr;
  pthread_attr_t attr_storage;

  attr = NULL;
  stack_size = thread_stack_size();

  if (stack_size > 0) {
    attr = &attr_storage;

    if (pthread_attr_init(attr))
      abort();

    if (pthread_attr_setstacksize(attr, stack_size))
      abort();
  }

  err = pthread_create(tid, attr, (void*(*)(void*)) entry, arg);

  if (attr != NULL)
    pthread_attr_destroy(attr);

  return -err;
}


uv_thread_t uv_thread_self(void) {
  return pthread_self();
}

int uv_thread_join(uv_thread_t *tid) {
  return -pthread_join(*tid, NULL);
}


int uv_thread_equal(const uv_thread_t* t1, const uv_thread_t* t2) {
  return pthread_equal(*t1, *t2);
}


int uv_mutex_init(uv_mutex_t* mutex) {
#if defined(NDEBUG) || !defined(PTHREAD_MUTEX_ERRORCHECK)
  return -pthread_mutex_init(mutex, NULL);
#else
  pthread_mutexattr_t attr;
  int err;

  if (pthread_mutexattr_init(&attr))
    abort();

  if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK))
    abort();

  err = pthread_mutex_init(mutex, &attr);

  if (pthread_mutexattr_destroy(&attr))
    abort();

  return -err;
#endif
}


int uv_mutex_init_recursive(uv_mutex_t* mutex) {
  pthread_mutexattr_t attr;
  int err;

  if (pthread_mutexattr_init(&attr))
    abort();

  if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE))
    abort();

  err = pthread_mutex_init(mutex, &attr);

  if (pthread_mutexattr_destroy(&attr))
    abort();

  return -err;
}


void uv_mutex_destroy(uv_mutex_t* mutex) {
  if (pthread_mutex_destroy(mutex))
    abort();
}


void uv_mutex_lock(uv_mutex_t* mutex) {
  if (pthread_mutex_lock(mutex))
    abort();
}


int uv_mutex_trylock(uv_mutex_t* mutex) {
  int err;

  err = pthread_mutex_trylock(mutex);
  if (err) {
    if (err != EBUSY && err != EAGAIN)
      abort();
    return -EBUSY;
  }

  return 0;
}


void uv_mutex_unlock(uv_mutex_t* mutex) {
  if (pthread_mutex_unlock(mutex))
    abort();
}


int uv_rwlock_init(uv_rwlock_t* rwlock) {
  return -pthread_rwlock_init(rwlock, NULL);
}


void uv_rwlock_destroy(uv_rwlock_t* rwlock) {
  if (pthread_rwlock_destroy(rwlock))
    abort();
}


void uv_rwlock_rdlock(uv_rwlock_t* rwlock) {
  if (pthread_rwlock_rdlock(rwlock))
    abort();
}


int uv_rwlock_tryrdlock(uv_rwlock_t* rwlock) {
  int err;

  err = pthread_rwlock_tryrdlock(rwlock);
  if (err) {
    if (err != EBUSY && err != EAGAIN)
      abort();
    return -EBUSY;
  }

  return 0;
}


void uv_rwlock_rdunlock(uv_rwlock_t* rwlock) {
  if (pthread_rwlock_unlock(rwlock))
    abort();
}


void uv_rwlock_wrlock(uv_rwlock_t* rwlock) {
  if (pthread_rwlock_wrlock(rwlock))
    abort();
}


int uv_rwlock_trywrlock(uv_rwlock_t* rwlock) {
  int err;

  err = pthread_rwlock_trywrlock(rwlock);
  if (err) {
    if (err != EBUSY && err != EAGAIN)
      abort();
    return -EBUSY;
  }

  return 0;
}


void uv_rwlock_wrunlock(uv_rwlock_t* rwlock) {
  if (pthread_rwlock_unlock(rwlock))
    abort();
}


void uv_once(uv_once_t* guard, void (*callback)(void)) {
  if (pthread_once(guard, callback))
    abort();
}

#if defined(__APPLE__) && defined(__MACH__)

int uv_sem_init(uv_sem_t* sem, unsigned int value) {
  kern_return_t err;

  err = semaphore_create(mach_task_self(), sem, SYNC_POLICY_FIFO, value);
  if (err == KERN_SUCCESS)
    return 0;
  if (err == KERN_INVALID_ARGUMENT)
    return -EINVAL;
  if (err == KERN_RESOURCE_SHORTAGE)
    return -ENOMEM;

  abort();
  return -EINVAL;  /* Satisfy the compiler. */
}


void uv_sem_destroy(uv_sem_t* sem) {
  if (semaphore_destroy(mach_task_self(), *sem))
    abort();
}


void uv_sem_post(uv_sem_t* sem) {
  if (semaphore_signal(*sem))
    abort();
}


void uv_sem_wait(uv_sem_t* sem) {
  int r;

  do
    r = semaphore_wait(*sem);
  while (r == KERN_ABORTED);

  if (r != KERN_SUCCESS)
    abort();
}


int uv_sem_trywait(uv_sem_t* sem) {
  mach_timespec_t interval;
  kern_return_t err;

  interval.tv_sec = 0;
  interval.tv_nsec = 0;

  err = semaphore_timedwait(*sem, interval);
  if (err == KERN_SUCCESS)
    return 0;
  if (err == KERN_OPERATION_TIMED_OUT)
    return -EAGAIN;

  abort();
  return -EINVAL;  /* Satisfy the compiler. */
}

#elif defined(__MVS__)

int uv_sem_init(uv_sem_t* sem, unsigned int value) {
  uv_sem_t semid;
  int err;
  union {
    int val;
    struct semid_ds* buf;
    unsigned short* array;
  } arg;


  semid = semget(IPC_PRIVATE, 1, S_IRUSR | S_IWUSR);
  if (semid == -1)
    return -errno;

  arg.val = value;
  if (-1 == semctl(semid, 0, SETVAL, arg)) {
    err = errno;
    if (-1 == semctl(*sem, 0, IPC_RMID))
      abort();
    return -err;
  }

  *sem = semid;
  return 0;
}

void uv_sem_destroy(uv_sem_t* sem) {
  if (-1 == semctl(*sem, 0, IPC_RMID))
    abort();
}

void uv_sem_post(uv_sem_t* sem) {
  struct sembuf buf;

  buf.sem_num = 0;
  buf.sem_op = 1;
  buf.sem_flg = 0;

  if (-1 == semop(*sem, &buf, 1))
    abort();
}

void uv_sem_wait(uv_sem_t* sem) {
  struct sembuf buf;
  int op_status;

  buf.sem_num = 0;
  buf.sem_op = -1;
  buf.sem_flg = 0;

  do
    op_status = semop(*sem, &buf, 1);
  while (op_status == -1 && errno == EINTR);

  if (op_status)
    abort();
}

int uv_sem_trywait(uv_sem_t* sem) {
  struct sembuf buf;
  int op_status;

  buf.sem_num = 0;
  buf.sem_op = -1;
  buf.sem_flg = IPC_NOWAIT;

  do
    op_status = semop(*sem, &buf, 1);
  while (op_status == -1 && errno == EINTR);

  if (op_status) {
    if (errno == EAGAIN)
      return -EAGAIN;
    abort();
  }

  return 0;
}

#else /* !(defined(__APPLE__) && defined(__MACH__)) */

int uv_sem_init(uv_sem_t* sem, unsigned int value) {
  if (sem_init(sem, 0, value))
    return -errno;
  return 0;
}


void uv_sem_destroy(uv_sem_t* sem) {
  if (sem_destroy(sem))
    abort();
}


void uv_sem_post(uv_sem_t* sem) {
  if (sem_post(sem))
    abort();
}


void uv_sem_wait(uv_sem_t* sem) {
  int r;

  do
    r = sem_wait(sem);
  while (r == -1 && errno == EINTR);

  if (r)
    abort();
}


int uv_sem_trywait(uv_sem_t* sem) {
  int r;

  do
    r = sem_trywait(sem);
  while (r == -1 && errno == EINTR);

  if (r) {
    if (errno == EAGAIN)
      return -EAGAIN;
    abort();
  }

  return 0;
}

#endif /* defined(__APPLE__) && defined(__MACH__) */


#if defined(__APPLE__) && defined(__MACH__) || defined(__MVS__)

int uv_cond_init(uv_cond_t* cond) {
  return -pthread_cond_init(cond, NULL);
}

#else /* !(defined(__APPLE__) && defined(__MACH__)) */

int uv_cond_init(uv_cond_t* cond) {
  pthread_condattr_t attr;
  int err;

  err = pthread_condattr_init(&attr);
  if (err)
    return -err;

#if !(defined(__ANDROID_API__) && __ANDROID_API__ < 21)
  err = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
  if (err)
    goto error2;
#endif

  err = pthread_cond_init(cond, &attr);
  if (err)
    goto error2;

  err = pthread_condattr_destroy(&attr);
  if (err)
    goto error;

  return 0;

error:
  pthread_cond_destroy(cond);
error2:
  pthread_condattr_destroy(&attr);
  return -err;
}

#endif /* defined(__APPLE__) && defined(__MACH__) */

void uv_cond_destroy(uv_cond_t* cond) {
#if defined(__APPLE__) && defined(__MACH__)
  /* It has been reported that destroying condition variables that have been
   * signalled but not waited on can sometimes result in application crashes.
   * See https://codereview.chromium.org/1323293005.
   */
  pthread_mutex_t mutex;
  struct timespec ts;
  int err;

  if (pthread_mutex_init(&mutex, NULL))
    abort();

  if (pthread_mutex_lock(&mutex))
    abort();

  ts.tv_sec = 0;
  ts.tv_nsec = 1;

  err = pthread_cond_timedwait_relative_np(cond, &mutex, &ts);
  if (err != 0 && err != ETIMEDOUT)
    abort();

  if (pthread_mutex_unlock(&mutex))
    abort();

  if (pthread_mutex_destroy(&mutex))
    abort();
#endif /* defined(__APPLE__) && defined(__MACH__) */

  if (pthread_cond_destroy(cond))
    abort();
}

void uv_cond_signal(uv_cond_t* cond) {
  if (pthread_cond_signal(cond))
    abort();
}

void uv_cond_broadcast(uv_cond_t* cond) {
  if (pthread_cond_broadcast(cond))
    abort();
}

void uv_cond_wait(uv_cond_t* cond, uv_mutex_t* mutex) {
  if (pthread_cond_wait(cond, mutex))
    abort();
}


int uv_cond_timedwait(uv_cond_t* cond, uv_mutex_t* mutex, uint64_t timeout) {
  int r;
  struct timespec ts;

#if defined(__APPLE__) && defined(__MACH__)
  ts.tv_sec = timeout / NANOSEC;
  ts.tv_nsec = timeout % NANOSEC;
  r = pthread_cond_timedwait_relative_np(cond, mutex, &ts);
#else
  timeout += uv__hrtime(UV_CLOCK_PRECISE);
  ts.tv_sec = timeout / NANOSEC;
  ts.tv_nsec = timeout % NANOSEC;
#if defined(__ANDROID_API__) && __ANDROID_API__ < 21

  /*
   * The bionic pthread implementation doesn't support CLOCK_MONOTONIC,
   * but has this alternative function instead.
   */
  r = pthread_cond_timedwait_monotonic_np(cond, mutex, &ts);
#else
  r = pthread_cond_timedwait(cond, mutex, &ts);
#endif /* __ANDROID_API__ */
#endif


  if (r == 0)
    return 0;

  if (r == ETIMEDOUT)
    return -ETIMEDOUT;

  abort();
  return -EINVAL;  /* Satisfy the compiler. */
}


int uv_barrier_init(uv_barrier_t* barrier, unsigned int count) {
  return -pthread_barrier_init(barrier, NULL, count);
}


void uv_barrier_destroy(uv_barrier_t* barrier) {
  if (pthread_barrier_destroy(barrier))
    abort();
}


int uv_barrier_wait(uv_barrier_t* barrier) {
  int r = pthread_barrier_wait(barrier);
  if (r && r != PTHREAD_BARRIER_SERIAL_THREAD)
    abort();
  return r == PTHREAD_BARRIER_SERIAL_THREAD;
}


int uv_key_create(uv_key_t* key) {
  return -pthread_key_create(key, NULL);
}


void uv_key_delete(uv_key_t* key) {
  if (pthread_key_delete(*key))
    abort();
}


void* uv_key_get(uv_key_t* key) {
  return pthread_getspecific(*key);
}


void uv_key_set(uv_key_t* key, void* value) {
  if (pthread_setspecific(*key, value))
    abort();
}
