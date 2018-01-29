/**
* @file Texture.cpp
*/
#include "Texture.h"
#include "DXGIFormat.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <stdio.h>
#include <sys/stat.h>

/**
* FOURCCを作成する.
*/
#define MAKE_FOURCC(a, b, c, d) static_cast<uint32_t>(a + (b << 8) + (c << 16) + (d << 24))

/**
* バイト列から数値を復元する.
*
* @param p      バイト列へのポインタ.
* @param offset 数値のオフセット.
* @param size   数値のバイト数(1～4).
*
* @return 復元した数値.
*/
uint32_t Get(const uint8_t* p, size_t offset, size_t size)
{
  uint32_t n = 0;
  p += offset;
  for (size_t i = 0; i < size; ++i) {
    n += p[i] << (i * 8);
  }
  return n;
}

/**
* DDS画像情報.
*/
struct DDSPixelFormat
{
  uint32_t size; ///< この構造体のバイト数(32).
  uint32_t flgas; ///< 画像に含まれるデータの種類を示すフラグ.
  uint32_t fourCC; ///< 画像フォーマットを示すFOURCC.
  uint32_t rgbBitCount; ///< 1ピクセルのビット数.
  uint32_t redBitMask; ///< 赤要素が使う部分を示すビット.
  uint32_t greenBitMask; ///< 緑要素が使う部分を示すビット.
  uint32_t blueBitMask; ///< 青要素が使う部分を示すビット.
  uint32_t alphaBitMask; ///< 透明要素が使う部分を示すビット.
};

/**
* バッファからDDS画像情報を読み出す.
*
* @param buf 読み出し元バッファ.
*
* @return 読み出したDDS画像情報.
*/
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

/**
* DDSファイルヘッダ.
*/
struct DDSHeader
{
  uint32_t size;  ///< この構造体のバイト数(124).
  uint32_t flags; ///< どのパラメータが有効かを示すフラグ.
  uint32_t height; ///< 画像の高さ(ピクセル数).
  uint32_t width; ///< 画像の幅(ピクセル数).
  uint32_t pitchOrLinearSize; ///< 横のバイト数または画像1枚のバイト数.
  uint32_t depth; ///< 画像の奥行き(枚数)(3次元テクスチャ等で使用).
  uint32_t mipMapCount; ///< 含まれているミップマップレベル数.
  uint32_t reserved1[11]; ///< (将来のために予約されている).
  DDSPixelFormat ddspf; ///< DDS画像情報.
  uint32_t caps[4]; ///< 含まれている画像の種類.
  uint32_t reserved2; ///< (将来のために予約されている).
};

/**
* バッファからDDSファイルヘッダを読み出す.
*
* @param buf 読み出し元バッファ.
*
* @return 読み出したDDSファイルヘッダ.
*/
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

/**
* DDSファイルヘッダ(DirectX10拡張).
*/
struct DDSHeaderDX10
{
  uint32_t dxgiFormat; ///< 画像の種類(DX10以降に追加された種類).
  uint32_t resourceDimension; ///< 次元数(1D or 2D or 3D).
  uint32_t miscFlag; ///< 画像が想定する使われ方を示すフラグ.
  uint32_t arraySize; ///< 格納されているのがテクスチャ配列の場合、その配列サイズ.
  uint32_t reserved; ///< (将来のために予約されている).
};

/**
* バッファからDDSファイルヘッダ(DirectX10拡張)を読み出す.
*
* @param buf 読み出し元バッファ.
*
* @return 読み出したDDSファイルヘッダ(DirectX10拡張).
*/
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

/**
* DDSファイルからテクスチャを作成する.
*
* @param filename DDSファイル名.
* @param st       DDSファイルステータス.
* @param buf      ファイルを読み込んだバッファ.
* @param header   DDSヘッダ格納先へのポインタ.
*
* @retval 0以外 作成したテクスチャID.
* @retval 0     作成失敗.
*/
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
  GLenum iformat;
  GLenum format = GL_RGBA;
  size_t imageOffset = 128;
  uint32_t blockSize = 16;
  bool isCompressed = false;
  if (header.ddspf.flgas & 0x04) {
    switch (header.ddspf.fourCC) {
    case MAKE_FOURCC('D', 'X', 'T', '1'):
      iformat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
      blockSize = 8;
      break;
    case MAKE_FOURCC('D', 'X', 'T', '2'):
    case MAKE_FOURCC('D', 'X', 'T', '3'):
      iformat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
      break;
    case MAKE_FOURCC('D', 'X', 'T', '4'):
    case MAKE_FOURCC('D', 'X', 'T', '5'):
      iformat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
      break;
    case MAKE_FOURCC('B', 'C', '4', 'U'):
      iformat = GL_COMPRESSED_RED_RGTC1;
      break;
    case MAKE_FOURCC('B', 'C', '4', 'S'):
      iformat = GL_COMPRESSED_SIGNED_RED_RGTC1;
      break;
    case MAKE_FOURCC('B', 'C', '5', 'U'):
      iformat = GL_COMPRESSED_RG_RGTC2;
      break;
    case MAKE_FOURCC('B', 'C', '5', 'S'):
      iformat = GL_COMPRESSED_SIGNED_RG_RGTC2;
      break;
    case MAKE_FOURCC('D', 'X', '1', '0'):
    {
      const DDSHeaderDX10 headerDX10 = ReadDDSHeaderDX10(buf + 128);
      switch (headerDX10.dxgiFormat) {
      case DXGI_FORMAT_BC1_UNORM: iformat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; blockSize = 8; break;
      case DXGI_FORMAT_BC2_UNORM: iformat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; break;
      case DXGI_FORMAT_BC3_UNORM: iformat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; break;
      case DXGI_FORMAT_BC1_UNORM_SRGB: iformat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT; blockSize = 8; break;
      case DXGI_FORMAT_BC2_UNORM_SRGB: iformat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT; break;
      case DXGI_FORMAT_BC3_UNORM_SRGB: iformat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT; break;
      case DXGI_FORMAT_BC4_UNORM: iformat = GL_COMPRESSED_RED_RGTC1; break;
      case DXGI_FORMAT_BC4_SNORM: iformat = GL_COMPRESSED_SIGNED_RED_RGTC1; break;
      case DXGI_FORMAT_BC5_UNORM: iformat = GL_COMPRESSED_RG_RGTC2; break;
      case DXGI_FORMAT_BC5_SNORM: iformat = GL_COMPRESSED_SIGNED_RG_RGTC2; break;
      case DXGI_FORMAT_BC6H_UF16: iformat = GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT; break;
      case DXGI_FORMAT_BC6H_SF16: iformat = GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT; break;
      case DXGI_FORMAT_BC7_UNORM: iformat = GL_COMPRESSED_RGBA_BPTC_UNORM; break;
      case DXGI_FORMAT_BC7_UNORM_SRGB: iformat = GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM; break;
      default:
        std::cerr << "WARNING: " << filename << "は未対応のDDSファイルです." << std::endl;
        return 0;
      }
      imageOffset = 128 + 20; // DX10ヘッダのぶんを加算.
      break;
    }
    default:
      std::cerr << "WARNING: " << filename << "は未対応のDDSファイルです." << std::endl;
      return 0;
    }
    isCompressed = true;
  } else if (header.ddspf.flgas & 0x40) {
    if (header.ddspf.redBitMask == 0xff) {
      iformat = header.ddspf.alphaBitMask ? GL_RGBA8 : GL_RGB8;
      format = header.ddspf.alphaBitMask ? GL_RGBA : GL_RGB;
    } else if (header.ddspf.blueBitMask == 0xff) {
      iformat = header.ddspf.alphaBitMask ? GL_RGBA8 : GL_RGB8;
      format = header.ddspf.alphaBitMask ? GL_BGRA : GL_BGR;
    }
  } else {
    std::cerr << "WARNING: " << filename << "は未対応のDDSファイルです." << std::endl;
    return 0;
  }

  const bool isCubemap = header.caps[1] & 0x200;
  const GLenum target = isCubemap ? GL_TEXTURE_CUBE_MAP_POSITIVE_X : GL_TEXTURE_2D;
  const int faceCount = isCubemap ? 6 : 1;

  GLuint texId;
  glGenTextures(1, &texId);
  glBindTexture(isCubemap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, texId);

  const uint8_t* data = buf + imageOffset;
  for (int faceIndex = 0; faceIndex < faceCount; ++faceIndex) {
    GLsizei curWidth = header.width;
    GLsizei curHeight = header.height;
    for (int mipLevel = 0; mipLevel < static_cast<int>(header.mipMapCount); ++mipLevel) {
      uint32_t imageSizeWithPadding;
      if (isCompressed) {
        imageSizeWithPadding = ((curWidth + 3) / 4) * ((curHeight + 3) / 4) * blockSize;
      } else {
        imageSizeWithPadding = curWidth * curHeight * 4;
      }
      if (isCompressed) {
        glCompressedTexImage2D(target + faceIndex, mipLevel, iformat, curWidth, curHeight, 0, imageSizeWithPadding, data);
      } else {
        glTexImage2D(target + faceIndex, mipLevel, iformat, curWidth, curHeight, 0, format, GL_UNSIGNED_BYTE, data);
      }
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
      data += imageSizeWithPadding;
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
    std::cerr << "ERROR in Texture::Create: 0x" << std::hex << result << std::endl;
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
    return {};
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
