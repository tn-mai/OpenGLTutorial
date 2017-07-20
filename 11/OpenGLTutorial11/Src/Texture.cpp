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
* �o�C�g�񂩂琔�l�𕜌�����.
*
* @param p      �o�C�g��ւ̃|�C���^.
* @param offset ���l�̃I�t�Z�b�g(�o�C�g).
* @param size   ���l�̃o�C�g��(1�`4).
*
* @return �����������l.
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
* �R���X�g���N�^.
*/
Texture::Texture() : texId(0)
{
}

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
* �t�@�C������2D�e�N�X�`����ǂݍ���.
*
* @param filename �t�@�C����.
*
* @return �쐬�ɐ��������ꍇ�̓e�N�X�`���|�C���^��Ԃ�.
*         ���s�����ꍇ��nullptr�Ԃ�.
*/
TexturePtr Texture::LoadFromFile(const char* filename, GLenum wrapMode)
{
  struct stat st;
  if (stat(filename, &st)) {
    std::cerr << "WARNING: " << filename << "���J���܂���." << std::endl;
    return {};
  }
  const size_t bmpFileHeaderSize = 14;
  const size_t windowsV1HeaderSize = 40;
  if (st.st_size <= bmpFileHeaderSize + windowsV1HeaderSize) {
    std::cerr << "WARNING: " << filename << "��BMP�t�@�C���ł͂���܂���." << std::endl;
    return {};
  }

  FILE* fp = fopen(filename, "rb");
  if (!fp) {
    std::cerr << "WARNING: " << filename << "���J���܂���." << std::endl;
    return {};
  }
  std::vector<uint8_t> buf;
  buf.resize(st.st_size);
  const size_t readSize = fread(buf.data(), 1, st.st_size, fp);
  fclose(fp);
  if (readSize != st.st_size) {
    std::cerr << "WARNING: " << filename << "�̓ǂݍ��݂Ɏ��s." << std::endl;
    return {};
  }

  const uint8_t* pHeader = buf.data();
  if (pHeader[0] != 'B' || pHeader[1] != 'M') {
    std::cerr << "WARNING: " << filename << "��BMP�t�@�C���ł͂���܂���." << std::endl;
    return {};
  }

  const size_t offsetBytes = Get(pHeader, 10, 4);
  const uint32_t infoSize = Get(pHeader, 14, 4);
  const uint32_t width = Get(pHeader, 18, 4);
  const uint32_t height = Get(pHeader, 22, 4);
  const uint32_t bitCount = Get(pHeader, 28, 2);
  const uint32_t compression = Get(pHeader, 30, 4);
  if (infoSize != windowsV1HeaderSize || bitCount != 24 || compression) {
    std::cerr << "WARNING: " << filename << "��24bit�����kBMP�t�@�C���ł͂���܂���." << std::endl;
    return {};
  }
  const size_t pixelBytes = bitCount / 8;
  const size_t actualHBytes = ((width * pixelBytes + 3) / 4) * 4;
  const size_t imageSize = actualHBytes * height;
  if (buf.size() < offsetBytes + imageSize) {
    std::cerr << "WARNING: " << filename << "�̃f�[�^�����Ă��܂�." << std::endl;
    return {};
  }
  GLint alignment;
  glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  TexturePtr p = Create(width, height, GL_RGB8, GL_BGR, buf.data() + offsetBytes, wrapMode);
  glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
  return p;
}
