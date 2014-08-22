#ifndef BASE_PATH_H_
#define BASE_PATH_H_

#include <string>

// join 2 paths. The result will be cleaned.
std::string joinPath(const std::string& lhs, const std::string& rhs);

bool isDirectory(const std::string& path);

#endif
