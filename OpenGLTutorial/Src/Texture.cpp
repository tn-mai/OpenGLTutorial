/**
* @file Texture.cpp
*/
#include "Texture.h"
#include <iostream>

/**
* �f�X�g���N�^.
*/
Texture::~Texture()
{
  if (texId) {
    glDeleteTextures(1, &texId);
  }
}

/**
* 2D�e�N�X�`�����쐬����.
*
* @param width   �e�N�X�`���̕�(�s�N�Z����).
* @param height  �e�N�X�`���̍���(�s�N�Z����).
* @param iformat �e�N�X�`���̃f�[�^�`��.
* @param format  �A�N�Z�X����v�f.
* @param data    �e�N�X�`���f�[�^�ւ̃|�C���^.
*
* @return �쐬�ɐ��������ꍇ�̓e�N�X�`���|�C���^��Ԃ�.
*         ���s�����ꍇ��nullptr�Ԃ�.
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
    std::cerr << "ERROR �e�N�X�`���쐬�Ɏ��s: 0x" << std::hex << result << std::endl;
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
