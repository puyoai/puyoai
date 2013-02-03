#ifndef CPU_PERIA_BASE_H_
#define CPU_PERIA_BASE_H_

#include <cstddef>
#include <cassert>
#include <cstdlib>
#include <string>

namespace std {}
using namespace std;

// scoped_ptr copied from https://code.google.com/searchframe#OAMlx_jo-ck/src/googleurl/base/scoped_ptr.h
template <typename T>
class scoped_ptr {
 private:
  T* ptr;
  scoped_ptr(scoped_ptr const &);
  scoped_ptr & operator=(scoped_ptr const &);

 public:
  typedef T element_type;
  explicit scoped_ptr(T* p = 0) : ptr(p) {}
  ~scoped_ptr() {
    typedef char type_must_be_complete[sizeof(T)];
    delete ptr;
  }
  void reset(T* p = 0) {
    typedef char type_must_be_complete[sizeof(T)];
    if (ptr != p) {
      delete ptr;
      ptr = p;
    }
  }
  T& operator*() const {
    assert(ptr != 0);
    return *ptr;
  }
  T* operator->() const {
    assert(ptr != 0);
    return ptr;
  }
  bool operator==(T* p) const { return ptr == p; }
  bool operator!=(T* p) const { return ptr != p; }
  T* get() const  { return ptr; }
  void swap(scoped_ptr & b) {
    T* tmp = b.ptr;
    b.ptr = ptr;
    ptr = tmp;
  }
  T* release() {
    T* tmp = ptr;
    ptr = 0;
    return tmp;
  }
};

string StringPrintf(const char* format, ...);

#endif  // CPU_PERIA_BASE_H_
