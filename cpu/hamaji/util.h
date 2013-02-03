#ifndef UTIL_H_
#define UTIL_H_

#include <string>
#include <vector>

#include "base.h"

string ssprintf(const char* fmt, ...);

void split(const string& str, const string& delim, vector<string>* output);

template <class T>
void delete_clear(vector<T*>* v) {
  for (typename vector<T*>::iterator iter = v->begin();
       iter != v->end();
       ++iter)
    delete *iter;
  v->clear();
}

#endif
