#ifndef BASE_FILE_FILE_H_
#define BASE_FILE_FILE_H_

#include <string>
#include <vector>

namespace file {

// Reads file from |filename| and copy to |output|.
bool readFile(const std::string& filename, std::string* output);

// Writes |content| to |filename|.
bool writeFile(const std::string& filename, const std::string& content);

// Copies files.
bool copyFile(const std::string& src, const std::string& dest);

} // namespace file

#endif // BASE_FILE_FILE_H_
