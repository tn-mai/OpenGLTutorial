/**
* @file OffscreenBuffer.cpp
*/
#include "OffscreenBuffer.h"

/**
* オフスクリーンバッファを作成する.
*
* @param w オフスクリーンバッファの幅(ピクセル単位).
* @param h オフスクリーンバッファの高さ(ピクセル単位).
*
* @return 作成したオフスクリーンバッファへのポインタ.
*/
OffscreenBufferPtr OffscreenBuffer::Create(int w, int h, GLenum iformat)
{
  struct Impl : OffscreenBuffer {};
  OffscreenBufferPtr offscreen = std::make_shared<Impl>();

  offscreen->tex = Texture::Create(w, h, iformat, GL_RGBA, nullptr);
  if (!offscreen->tex) {
    return {};
  }

  glGenRenderbuffers(1, &offscreen->depthbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, offscreen->depthbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glGenFramebuffers(1, &offscreen->framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, offscreen->framebuffer);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, offscreen->depthbuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, offscreen->tex->Id(), 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return offscreen;
}

/**
* デストラクタ.
*/
OffscreenBuffer::~OffscreenBuffer()
{
  if (framebuffer) {
    glDeleteFramebuffers(1, &framebuffer);
  }
  if (depthbuffer) {
    glDeleteRenderbuffers(1, &depthbuffer);
  }
}
