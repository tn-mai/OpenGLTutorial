/**
* @file File.cpp
*/
#include "File.h"

namespace File {

/**
* �t�@�C����ǂݍ���.
*
* @param filename �ǂݍ��ރt�@�C����.
* @param buf      �ǂݍ��ݐ�o�b�t�@.
*
* @retval true �ǂݍ��ݐ���.
* @retval false �ǂݍ��ݎ��s.
*/
bool ReadFile(const char* filename, std::vector<char>& buf)
{
  struct stat st;
  if (stat(filename, &st)) {
    return false;
  }
  FILE* fp = fopen(filename, "rb");
  if (!fp) {
    return false;
  }
  buf.resize(st.st_size + 1);
  const size_t readSize = fread(buf.data(), 1, st.st_size, fp);
  fclose(fp);
  if (readSize != st.st_size) {
    return false;
  }
  buf.back() = '\0';
  return true;
}

} // namespace File