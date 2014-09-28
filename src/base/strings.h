#ifndef BASE_STRINGS_H_
#define BASE_STRINGS_H_

#include <string>

// When |s|'s prefix is prefix, true will be returned.
bool isPrefix(const std::string& s, const std::string& prefix);

// Remove heading and trailing spaces.
std::string trim(const std::string&);

#endif
