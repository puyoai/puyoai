/*
c-pthread-queue - c implementation of a bounded buffer queue using posix threads
Copyright (C) 2008  Matthew Dickinson

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <vector>
#include <pthread.h>
#include <stdio.h>

#ifndef _QUEUE_H
#define _QUEUE_H

#define QUEUE_INITIALIZER(buffer) { buffer, sizeof(buffer) / sizeof(buffer[0]), 0, 0, 0, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER }

template <class T>
class Queue {
 public:
	std::vector<T> buffer_;
	const int capacity;
	int size_;
	int in;
	int out;
	pthread_mutex_t mutex;
	pthread_cond_t cond_full;
	pthread_cond_t cond_empty;
	
	Queue(int cap) :
	    buffer_(cap), capacity(cap), size_(0), in(0), out(0),
	    mutex((pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER),
	    cond_full((pthread_cond_t)PTHREAD_COND_INITIALIZER),
	    cond_empty((pthread_cond_t)PTHREAD_COND_INITIALIZER) { }
	
  void enqueue(const T& value) {
  	pthread_mutex_lock(&(mutex));
  	while (size_ == capacity)
  		pthread_cond_wait(&(cond_full), &(mutex));
  	buffer_[in] = value;
  	++ size_;
  	++ in;
  	in %= capacity;
  	pthread_mutex_unlock(&(mutex));
  	pthread_cond_broadcast(&(cond_empty));
  }

  T dequeue() {
  	pthread_mutex_lock(&(mutex));
  	while (size_ == 0)
  		pthread_cond_wait(&(cond_empty), &(mutex));
  	T value = buffer_[out];
  	-- size_;
  	++ out;
  	out %= capacity;
  	pthread_mutex_unlock(&(mutex));
  	pthread_cond_broadcast(&(cond_full));
  	return value;
  }

  bool try_dequeue(T* value) {
  	pthread_mutex_lock(&(mutex));
  	if (size_ == 0) {
      pthread_mutex_unlock(&(mutex));
      return false;
    }
  	*value = buffer_[out];
  	-- size_;
  	++ out;
  	out %= capacity;
  	pthread_mutex_unlock(&(mutex));
  	pthread_cond_broadcast(&(cond_full));
  	return true;
  }

  int size() {
  	pthread_mutex_lock(&(mutex));
  	int size = size_;
  	pthread_mutex_unlock(&(mutex));
  	return size;
  }

};

#endif
