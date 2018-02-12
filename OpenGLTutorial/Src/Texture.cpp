/**
* @file Texture.cpp
*/
#include "Texture.h"
#include <iostream>

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
TexturePtr Texture::Create(
  int width, int height, GLenum iformat, GLenum format, const void* data)
{
  struct Impl : Texture {};
  TexturePtr p = std::make_shared<Impl>();

  p->width = width;
  p->height = height;
  glGenTextures(1, &p->texId);
  glBindTexture(GL_TEXTURE_2D, p->texId);
  glTexImage2D(GL_TEXTURE_2D, 0, iformat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
  const GLenum result = glGetError();
  if (result != GL_NO_ERROR) {
    std::cerr << "ERROR テクスチャ作成に失敗: 0x" << std::hex << result << std::endl;
    return {};
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glBindTexture(GL_TEXTURE_2D, 0);

  return p;
}
