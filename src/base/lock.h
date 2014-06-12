#ifndef BASE_LOCK_H_
#define BASE_LOCK_H_

#include <pthread.h>
#include <glog/logging.h>
#include "base/base.h"

// TODO(mayah): Now that we're using C++11, should we use <thread> library?

// Enable thread safety attributes only with clang.
// The attributes can be safely erased when compiling with other compilers.
#if defined(__clang__) && (!defined(SWIG))
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   __attribute__((x))
#else
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   // no-op
#endif

//#define THREAD_ANNOTATION_ATTRIBUTE__(x)   __attribute__((x))

#define GUARDED_BY(x) \
    THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

#define GUARDED_VAR \
    THREAD_ANNOTATION_ATTRIBUTE__(guarded)

#define PT_GUARDED_BY(x) \
    THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded_by(x))

#define PT_GUARDED_VAR \
    THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded)

#define ACQUIRED_AFTER(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(acquired_after(__VA_ARGS__))

#define ACQUIRED_BEFORE(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(acquired_before(__VA_ARGS__))

#define EXCLUSIVE_LOCKS_REQUIRED(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(exclusive_locks_required(__VA_ARGS__))

#define SHARED_LOCKS_REQUIRED(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(shared_locks_required(__VA_ARGS__))

#define LOCKS_EXCLUDED(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))

#define LOCK_RETURNED(x) \
    THREAD_ANNOTATION_ATTRIBUTE__(lock_returned(x))

#define LOCKABLE \
    THREAD_ANNOTATION_ATTRIBUTE__(lockable)

#define SCOPED_LOCKABLE \
    THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)

#define EXCLUSIVE_LOCK_FUNCTION(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(exclusive_lock_function(__VA_ARGS__))

#define SHARED_LOCK_FUNCTION(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(shared_lock_function(__VA_ARGS__))

#define ASSERT_EXCLUSIVE_LOCK(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(assert_exclusive_lock(__VA_ARGS__))

#define ASSERT_SHARED_LOCK(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_lock(__VA_ARGS__))

#define EXCLUSIVE_TRYLOCK_FUNCTION(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(exclusive_trylock_function(__VA_ARGS__))

#define SHARED_TRYLOCK_FUNCTION(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(shared_trylock_function(__VA_ARGS__))

#define UNLOCK_FUNCTION(...) \
    THREAD_ANNOTATION_ATTRIBUTE__(unlock_function(__VA_ARGS__))

#define NO_THREAD_SAFETY_ANALYSIS \
    THREAD_ANNOTATION_ATTRIBUTE__(no_thread_safety_analysis)

class LOCKABLE Mutex : noncopyable {
public:
    Mutex() { CHECK(pthread_mutex_init(&mu_, nullptr) == 0); }
    ~Mutex() { CHECK(pthread_mutex_destroy(&mu_) == 0); }

    void lock() EXCLUSIVE_LOCK_FUNCTION() { CHECK(pthread_mutex_lock(&mu_) == 0); }
    void unlock() UNLOCK_FUNCTION() { CHECK(pthread_mutex_unlock(&mu_) == 0); }

private:
    pthread_mutex_t mu_;
    friend class ConditionVariable;
};

class ConditionVariable : noncopyable {
public:
    ConditionVariable() { CHECK(pthread_cond_init(&cond_, nullptr) == 0); }
    ~ConditionVariable() { CHECK(pthread_cond_destroy(&cond_) == 0); }
    void wait(Mutex* mu) { CHECK(pthread_cond_wait(&cond_, &mu->mu_) == 0); }
    void signal() { CHECK(pthread_cond_signal(&cond_) == 0); }

private:
    pthread_cond_t cond_;
};

class SCOPED_LOCKABLE ScopedLock : noncopyable {
public:
    explicit ScopedLock(Mutex* mu) EXCLUSIVE_LOCK_FUNCTION(mu) : mu_(mu) { mu_->lock(); }
    ~ScopedLock() UNLOCK_FUNCTION() { mu_->unlock(); }

private:
    Mutex* mu_;
};

#endif
