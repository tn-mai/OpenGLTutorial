/**
* @file Texture.cpp
*/
#include "Texture.h"
#include "DXGIFormat.h"
#include <iostream>
#include <vector>
#include <cstdint>
#include <stdio.h>
#include <sys/stat.h>
#include <algorithm>

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

struct DDSPixelFormat
{
  uint32_t size;
  uint32_t flgas;
  uint32_t fourCC;
  uint32_t rgbBitCount;
  uint32_t redBitMask;
  uint32_t greenBitMask;
  uint32_t blueBitMask;
  uint32_t alphaBitMask;
};

DDSPixelFormat ReadDDSPixelFormat(const uint8_t* buf)
{
  DDSPixelFormat tmp;
  tmp.size = Get(buf, 0, 4);
  tmp.flgas = Get(buf, 4, 4);
  tmp.fourCC = Get(buf, 8, 4);
  tmp.rgbBitCount = Get(buf, 12, 4);
  tmp.redBitMask = Get(buf, 16, 4);
  tmp.greenBitMask = Get(buf, 20, 4);
  tmp.blueBitMask = Get(buf, 24, 4);
  tmp.alphaBitMask = Get(buf, 28, 4);
  return tmp;
}

struct DDSHeader
{
  uint32_t size;
  uint32_t flags;
  uint32_t height;
  uint32_t width;
  uint32_t pitchOrLinearSize;
  uint32_t depth;
  uint32_t mipMapCount;
  uint32_t reserved1[11];
  DDSPixelFormat ddspf;
  uint32_t caps[4];
  uint32_t reserved2;
};

DDSHeader ReadDDSHeader(const uint8_t* buf)
{
  DDSHeader tmp = {};
  tmp.size = Get(buf, 0, 4);
  tmp.flags = Get(buf, 4, 4);
  tmp.height = Get(buf, 8, 4);
  tmp.width = Get(buf, 12, 4);
  tmp.pitchOrLinearSize = Get(buf, 16, 4);
  tmp.depth = Get(buf, 20, 4);
  tmp.mipMapCount = Get(buf, 24, 4);
  tmp.ddspf = ReadDDSPixelFormat(buf + 28 + 4 * 11);
  for (int i = 0; i < 4; ++i) {
    tmp.caps[i] = Get(buf, 28 + 4 * 11 + 32 + i * 4, 4);
  }
  return tmp;
}

struct DDSHeaderDX10
{
  uint32_t dxgiFormat;
  uint32_t resourceDimension;
  uint32_t miscFlag;
  uint32_t arraySize;
  uint32_t reserved;
};
DDSHeaderDX10 ReadDDSHeaderDX10(const uint8_t* buf)
{
  DDSHeaderDX10 tmp;
  tmp.dxgiFormat = Get(buf, 0, 4);
  tmp.resourceDimension = Get(buf, 4, 4);
  tmp.miscFlag = Get(buf, 8, 4);
  tmp.arraySize = Get(buf, 12, 4);
  tmp.reserved = Get(buf, 16, 4);
  return tmp;
}

GLint CorrectFilter(int mipCount, GLint filter)
{
  if (mipCount > 1) {
    switch (filter) {
    case GL_NEAREST:
      return GL_NEAREST_MIPMAP_NEAREST;
    case GL_LINEAR:
    default:
      return GL_LINEAR_MIPMAP_LINEAR;
    case GL_NEAREST_MIPMAP_NEAREST:
    case GL_LINEAR_MIPMAP_NEAREST:
    case GL_NEAREST_MIPMAP_LINEAR:
    case GL_LINEAR_MIPMAP_LINEAR:
      return filter;
    }
  }
  return filter == GL_NEAREST ? GL_NEAREST : GL_LINEAR;
}

#define MAKE_FOURCC(a, b, c, d) static_cast<uint32_t>(a + (b << 8) + (c << 16) + (d << 24))

GLuint LoadDDS(const char* filename, const struct stat& st, const uint8_t* buf, DDSHeader* pHeader)
{
  if (st.st_size < 128) {
    std::cerr << "WARNING: " << filename << "はDDSファイルではありません." << std::endl;
    return 0;
  }

  const DDSHeader header = ReadDDSHeader(buf + 4);
  if (header.size != 124) {
    std::cerr << "WARNING: " << filename << "はDDSファイルではありません." << std::endl;
    return 0;
  }
  GLenum format;
  size_t imageOffset = 128;
  uint32_t blockSize = 16;
  switch (header.ddspf.fourCC) {
  case MAKE_FOURCC('D', 'X', 'T', '1'):
    format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
    blockSize = 8;
    break;
  case MAKE_FOURCC('D', 'X', 'T', '2'):
  case MAKE_FOURCC('D', 'X', 'T', '3'):
    format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    break;
  case MAKE_FOURCC('D', 'X', 'T', '4'):
  case MAKE_FOURCC('D', 'X', 'T', '5'):
    format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    break;
  case MAKE_FOURCC('B', 'C', '4', 'U'):
    format = GL_COMPRESSED_RED_RGTC1;
    break;
  case MAKE_FOURCC('B', 'C', '4', 'S'):
    format = GL_COMPRESSED_SIGNED_RED_RGTC1;
    break;
  case MAKE_FOURCC('B', 'C', '5', 'U'):
    format = GL_COMPRESSED_RG_RGTC2;
    break;
  case MAKE_FOURCC('B', 'C', '5', 'S'):
    format = GL_COMPRESSED_SIGNED_RG_RGTC2;
    break;
  case MAKE_FOURCC('D', 'X', '1', '0'): {
    const DDSHeaderDX10 headerDX10 = ReadDDSHeaderDX10(buf + 128);
    switch (headerDX10.dxgiFormat) {
    case DXGI_FORMAT_BC1_UNORM: format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; blockSize = 8; break;
    case DXGI_FORMAT_BC2_UNORM: format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; break;
    case DXGI_FORMAT_BC3_UNORM: format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
    case DXGI_FORMAT_BC1_UNORM_SRGB: format = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT; blockSize = 8; break;
    case DXGI_FORMAT_BC2_UNORM_SRGB: format = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT; break;
    case DXGI_FORMAT_BC3_UNORM_SRGB: format = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT; break;
    case DXGI_FORMAT_BC4_UNORM: format = GL_COMPRESSED_RED_RGTC1; break;
    case DXGI_FORMAT_BC4_SNORM: format = GL_COMPRESSED_SIGNED_RED_RGTC1; break;
    case DXGI_FORMAT_BC5_UNORM: format = GL_COMPRESSED_RG_RGTC2; break;
    case DXGI_FORMAT_BC5_SNORM: format = GL_COMPRESSED_SIGNED_RG_RGTC2; break;
    case DXGI_FORMAT_BC6H_UF16: format = GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT; break;
    case DXGI_FORMAT_BC6H_SF16: format = GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT; break;
    case DXGI_FORMAT_BC7_UNORM: format = GL_COMPRESSED_RGBA_BPTC_UNORM; break;
    case DXGI_FORMAT_BC7_UNORM_SRGB: format = GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM; break;
    default:
      std::cerr << "WARNING: " << filename << "は未対応のDDSファイルです." << std::endl;
      return 0;
    }
    imageOffset = 128 + 20;
    break;
  }
  default:
    std::cerr << "WARNING: " << filename << "は未対応のDDSファイルです." << std::endl;
    return 0;
  }

  const bool isCubemap = header.caps[1] & 0x200;
  const GLenum target = isCubemap ? GL_TEXTURE_CUBE_MAP_POSITIVE_X : GL_TEXTURE_2D;
  const int faceCount = isCubemap ? 6 : 1;

  GLuint texId;
  glGenTextures(1, &texId);
  glBindTexture(isCubemap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, texId);

  const uint8_t* pData = buf + imageOffset;
  for (int faceIndex = 0; faceIndex < faceCount; ++faceIndex) {
    GLsizei curWidth = header.width;
    GLsizei curHeight = header.height;
    for (int mipLevel = 0; mipLevel < static_cast<int>(header.mipMapCount); ++mipLevel) {
      const uint32_t imageSizeWithPadding = ((curWidth + 3) / 4) * ((curHeight + 3) / 4) * blockSize;
      glCompressedTexImage2D(target + faceIndex, mipLevel, format, curWidth, curHeight, 0, imageSizeWithPadding, pData);
      const GLenum result = glGetError();
      switch(result) {
      case GL_NO_ERROR:
        break;
      case GL_INVALID_OPERATION:
        std::cerr << "WARNING: " << filename << "の読み込みに失敗." << std::endl;
        break;
      default:
        std::cerr << "WARNING: " << filename << "の読み込みに失敗(" << std::hex << result << ")." << std::endl;
        break;
      }
      curWidth = std::max(1, curWidth / 2);
      curHeight = std::max(1, curHeight / 2);
      pData += imageSizeWithPadding;
    }
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, header.mipMapCount - 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, header.mipMapCount <= 1 ? GL_LINEAR : GL_LINEAR_MIPMAP_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glBindTexture(GL_TEXTURE_2D, 0);
  *pHeader = header;
  return texId;
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
  if (pHeader[0] == 'D' || pHeader[1] == 'D' || pHeader[2] == 'S' || pHeader[3] == ' ') {
    DDSHeader header;
    const GLuint texId = LoadDDS(filename, st, buf.data(), &header);
    if (texId) {
      struct impl : Texture {};
      TexturePtr p = std::make_shared<impl>();
      p->width = header.width;
      p->height = header.height;
      p->texId = texId;
      return p;
    }
  }
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
