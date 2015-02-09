#ifndef BASE_PATH_H_
#define BASE_PATH_H_

#include <string>
#include <vector>

namespace file {

// join 2 paths. The result will be cleaned.
std::string joinPath(const std::string& lhs, const std::string& rhs);

bool isDirectory(const std::string& path);

// Lists all files in path.
// true will be returned if suceeded, otherwise, false will be returned.
bool listFiles(const std::string& path, std::vector<std::string>* files);

} // namespace file

#endif
