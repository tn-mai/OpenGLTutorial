/**
* @file File.h
*/
#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED
#include <vector>

namespace File {

bool ReadFile(const char* filename, std::vector<char>& buf);

} // namespace File

#endif // FILE_H_INCLUDED