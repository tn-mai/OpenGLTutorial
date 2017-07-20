/**
* @file Texture.cpp
*/
#include "Texture.h"
#include <iostream>
#include <vector>
#include <cstdint>
#include <stdio.h>
#include <sys/stat.h>

/**
* バイト列から数値を復元する.
*
* @param p      バイト列へのポインタ.
* @param offset 数値のオフセット(バイト).
* @param size   数値のバイト数(1～4).
*
* @return 復元した数値.
*/
uint32_t Get(const uint8_t* p, size_t offset, size_t size)
{
  uint32_t n = 0;
  p += offset;
  for (size_t i = 0; i < size; ++i, ++p) {
    n += *p << (8 * i);
  }
  return n;
}

/**
* コンストラクタ.
*/
Texture::Texture() : texId(0)
{
}

/**
* デストラクタ.
*/
Texture::~Texture()
{
	if (texId) {
		glDeleteTextures(1, &texId);
	}
}

/**
* 2Dテクスチャを作成する.
*
* @param width   テクスチャの幅(ピクセル数).
* @param height  テクスチャの高さ(ピクセル数).
* @param iformat テクスチャのデータ形式.
* @param format  アクセスする要素.
* @param data    テクスチャデータへのポインタ.
*
* @return 作成に成功した場合はテクスチャポインタを返す.
*         失敗した場合はnullptr返す.
*/
TexturePtr Texture::Create(int width, int height, GLenum iformat, GLenum format, const void* data, GLenum wrapMode)
{
  GLenum type;
  switch (iformat) {
  case GL_RGB10_A2: type = GL_UNSIGNED_INT_2_10_10_10_REV; break;
  case GL_RGBA16F: type = GL_HALF_FLOAT; break;
  default: type = GL_UNSIGNED_BYTE;
  }
  struct impl : Texture {};
  TexturePtr p = std::make_shared<impl>();
  p->width = width;
  p->height = height;
  glGenTextures(1, &p->texId);
  glBindTexture(GL_TEXTURE_2D, p->texId);
  glTexImage2D(GL_TEXTURE_2D, 0, iformat, width, height, 0, format, type, data);
  const GLenum result = glGetError();
  if (result != GL_NO_ERROR) {
    std::cerr << "ERROR in Texture::Create(0x" << std::hex << result << ")" << std::endl;
    return {};
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
  glBindTexture(GL_TEXTURE_2D, 0);
  return p;
}

/**
* ファイルから2Dテクスチャを読み込む.
*
* @param filename ファイル名.
*
* @return 作成に成功した場合はテクスチャポインタを返す.
*         失敗した場合はnullptr返す.
*/
TexturePtr Texture::LoadFromFile(const char* filename, GLenum wrapMode)
{
  struct stat st;
  if (stat(filename, &st)) {
    std::cerr << "WARNING: " << filename << "が開けません." << std::endl;
    return {};
  }
  const size_t bmpFileHeaderSize = 14;
  const size_t windowsV1HeaderSize = 40;
  if (st.st_size <= bmpFileHeaderSize + windowsV1HeaderSize) {
    std::cerr << "WARNING: " << filename << "はBMPファイルではありません." << std::endl;
    return {};
  }

  FILE* fp = fopen(filename, "rb");
  if (!fp) {
    std::cerr << "WARNING: " << filename << "が開けません." << std::endl;
    return {};
  }
  std::vector<uint8_t> buf;
  buf.resize(st.st_size);
  const size_t readSize = fread(buf.data(), 1, st.st_size, fp);
  fclose(fp);
  if (readSize != st.st_size) {
    std::cerr << "WARNING: " << filename << "の読み込みに失敗." << std::endl;
    return {};
  }

  const uint8_t* pHeader = buf.data();
  if (pHeader[0] != 'B' || pHeader[1] != 'M') {
    std::cerr << "WARNING: " << filename << "はBMPファイルではありません." << std::endl;
    return {};
  }

  const size_t offsetBytes = Get(pHeader, 10, 4);
  const uint32_t infoSize = Get(pHeader, 14, 4);
  const uint32_t width = Get(pHeader, 18, 4);
  const uint32_t height = Get(pHeader, 22, 4);
  const uint32_t bitCount = Get(pHeader, 28, 2);
  const uint32_t compression = Get(pHeader, 30, 4);
  if (infoSize != windowsV1HeaderSize || bitCount != 24 || compression) {
    std::cerr << "WARNING: " << filename << "は24bit無圧縮BMPファイルではありません." << std::endl;
    return {};
  }
  const size_t pixelBytes = bitCount / 8;
  const size_t actualHBytes = ((width * pixelBytes + 3) / 4) * 4;
  const size_t imageSize = actualHBytes * height;
  if (buf.size() < offsetBytes + imageSize) {
    std::cerr << "WARNING: " << filename << "のデータが壊れています." << std::endl;
    return {};
  }
  GLint alignment;
  glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  TexturePtr p = Create(width, height, GL_RGB8, GL_BGR, buf.data() + offsetBytes, wrapMode);
  glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
  return p;
}
