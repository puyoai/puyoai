#ifndef BASE_FILE_PATH_H_
#define BASE_FILE_PATH_H_

#include <string>
#include <vector>

namespace file {

// join 2 paths. The result will be cleaned.
// TODO(mayah): Use variadic template.
std::string joinPath(const std::string& lhs, const std::string& rhs);
std::string joinPath(const std::string& p1, const std::string& p2, const std::string& p3);
std::string joinPath(const std::string& p1, const std::string& p2, const std::string& p3, const std::string& p4);

bool isDirectory(const std::string& path);
bool exists(const std::string& path);
bool isAbsolutePath(const std::string& path);

// Lists all files in path.
// true will be returned if suceeded, otherwise, false will be returned.
bool listFiles(const std::string& path, std::vector<std::string>* files);

// Removes a file.
// Returns true if succeeded.
bool remove(const std::string& filename);

std::string basename(std::string path);
std::string dirname(std::string path);
std::string stem(std::string path);
std::string extension(std::string path);

} // namespace file

#endif // BASE_FILE_PATH_H_
