/**
* @file File.cpp
*/
#include "File.h"

namespace File {

/**
* ファイルを読み込む.
*
* @param filename 読み込むファイル名.
* @param buf      読み込み先バッファ.
*
* @retval true 読み込み成功.
* @retval false 読み込み失敗.
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