#ifndef HAMAJI_MUTEX_H_
#define HAMAJI_MUTEX_H_

#include <pthread.h>

class Mutex {
public:
  Mutex() {
    pthread_mutex_init(&mu_, NULL);
  }
  ~Mutex() {
    pthread_mutex_destroy(&mu_);
  }

  void lock() {
    pthread_mutex_lock(&mu_);
  }
  void unlock() {
    pthread_mutex_unlock(&mu_);
  }

  pthread_mutex_t* mu() { return &mu_; }

private:
  pthread_mutex_t mu_;
};

class MutexLock {
public:
  explicit MutexLock(Mutex* mu)
    : mu_(mu) {
    mu_->lock();
  }
  ~MutexLock() {
    mu_->unlock();
  }

private:
  Mutex* mu_;
};

#endif  // HAMAJI_MUTEX_H_
