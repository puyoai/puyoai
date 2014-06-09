#include "util.h"

#include <stdio.h>
#include <stdarg.h>

string ssprintf(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  char buf[4097];
  int len = vsnprintf(buf, 4096, fmt, ap);
  buf[len] = '\0';
  va_end(ap);
  return buf;
}

void split(const string& str, const string& delim, vector<string>* output) {
  size_t prev = 0;
  while (true) {
    size_t found = str.find(delim, prev);
    output->push_back(str.substr(prev, found - prev));
    if (found == string::npos) {
      break;
    }
    prev = found + delim.size();
  }
}
