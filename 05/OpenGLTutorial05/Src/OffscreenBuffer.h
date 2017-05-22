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
  GLuint GetFramebuffer() const { return framebuffer; }
  GLuint GetTexutre() const { return tex->Id(); }

private:
  OffscreenBuffer() = default;
  OffscreenBuffer(const OffscreenBuffer&) = delete;
  OffscreenBuffer& operator=(const OffscreenBuffer&) = delete;
  ~OffscreenBuffer();

private:
  TexturePtr tex;
  GLuint depthbuffer = 0;
  GLuint framebuffer = 0;
};

#endif // OFFSCREENBUFFER_H_INCLUDED