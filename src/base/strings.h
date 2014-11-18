#ifndef BASE_STRINGS_H_
#define BASE_STRINGS_H_

#include <string>
#include <vector>

namespace strings {

// When |s|'s prefix is prefix, true will be returned.
bool isPrefix(const std::string& s, const std::string& prefix);

// Remove heading and trailing spaces.
std::string trim(const std::string& s);

std::vector<std::string> split(const std::string& s, char separator);

}

#endif
