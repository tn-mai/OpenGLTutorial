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
* �I�t�X�N���[���o�b�t�@.
*/
class OffscreenBuffer
{
public:
  static OffscreenBufferPtr Create(int w, int h);
  GLuint GetFramebuffer() const { return framebuffer; } ///< �t���[���o�b�t�@���擾����.
  GLuint GetTexutre() const { return tex->Id(); } ///< �t���[���o�b�t�@�p�e�N�X�`�����擾����.

private:
  OffscreenBuffer() = default;
  OffscreenBuffer(const OffscreenBuffer&) = delete;
  OffscreenBuffer& operator=(const OffscreenBuffer&) = delete;
  ~OffscreenBuffer();

private:
  TexturePtr tex; ///< �t���[���o�b�t�@�p�e�N�X�`��.
  GLuint depthbuffer = 0; ///< �[�x�o�b�t�@�I�u�W�F�N�g.
  GLuint framebuffer = 0; ///< �t���[���o�b�t�@�I�u�W�F�N�g.
};

#endif
