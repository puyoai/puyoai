#ifndef BASE_FILE_FILE_H_
#define BASE_FILE_FILE_H_

#include <string>
#include <vector>

namespace file {

// Reads file from |filename| and copy to |output|.
bool readFile(const std::string& filename, std::string* output);

} // namespace file

#endif // BASE_FILE_FILE_H_
