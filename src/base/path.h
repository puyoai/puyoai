#ifndef BASE_PATH_H_
#define BASE_PATH_H_

#include <string>

// When |s|'s prefix is prefix, true will be returned.
bool isPrefix(const std::string& s, const std::string& prefix);

// join 2 paths. The result will be cleaned.
std::string joinPath(const std::string& lhs, const std::string& rhs);

bool isDirectory(const std::string& path);

#endif
