/**
* @file OffscreenBuffer.h
*/
#ifndef OFFSCREENBUFFER_H_INCLUDED
#define OFFSCREENBUFFER_H_INCLUDED
#include "Texture.h"
#include <GL/glew.h>
#include <memory>

class OffscreenBuffer;
typedef std::shared_ptr<OffscreenBuffer> OffscreenBufferPtr;

/**
* オフスクリーンバッファ.
*/
class OffscreenBuffer
{
public:
  static OffscreenBufferPtr Create(int w, int h);
  GLuint GetFramebuffer() const { return framebuffer; } ///< フレームバッファを取得する.
  GLuint GetTexutre() const { return tex->Id(); } ///< フレームバッファ用テクスチャを取得する.

private:
  OffscreenBuffer() = default;
  OffscreenBuffer(const OffscreenBuffer&) = delete;
  OffscreenBuffer& operator=(const OffscreenBuffer&) = delete;
  ~OffscreenBuffer();

private:
  TexturePtr tex; ///< フレームバッファ用テクスチャ.
  GLuint depthbuffer = 0; ///< 深度バッファオブジェクト.
  GLuint framebuffer = 0; ///< フレームバッファオブジェクト.
};

#endif
