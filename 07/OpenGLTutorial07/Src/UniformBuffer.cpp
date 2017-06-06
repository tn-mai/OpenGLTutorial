/**
* @file UniformBuffer.cpp
*/
#include "UniformBuffer.h"
#include <iostream>

/**
* Uniform�o�b�t�@���쐬����.
*
* @param size         �o�b�t�@�̃o�C�g�T�C�Y.
* @param bindingPoint �o�b�t�@�����蓖�Ă�o�C���f�B���O�E�|�C���g.
* @param name         �o�b�t�@��(�f�o�b�O�p).
*
* @return �쐬����Uniform�o�b�t�@�ւ̃|�C���^.
*/
UniformBufferPtr UniformBuffer::Create(GLsizeiptr size, GLuint bindingPoint ,const char* name)
{
  struct Impl : UniformBuffer { Impl() {} ~Impl() {} };
  UniformBufferPtr p = std::make_shared<Impl>();
  if (!p) {
    std::cerr << "ERROR: UBO '" << name << "'�̍쐬�Ɏ��s" << std::endl;
    return {};
  }

  glGenBuffers(1, &p->ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, p->ubo);
  glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, p->ubo);
  const GLenum result = glGetError();
  if (result != GL_NO_ERROR) {
    std::cerr << "ERROR: UBO '" << name << "'�̍쐬�Ɏ��s" << std::endl;
    return {};
  }
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  p->size = size;
  p->bindingPoint = bindingPoint;
  p->name = name;
  return p;
}

/**
* �f�X�g���N�^.
*/
UniformBuffer::~UniformBuffer()
{
  if (ubo) {
    glDeleteBuffers(1, &ubo);
  }
}

/**
* Uniform�o�b�t�@�Ƀf�[�^��]������.
*
* @param data   �]������f�[�^�ւ̃|�C���^.
* @param offset �]����̃o�C�g�I�t�Z�b�g.
* @param size   �]������o�C�g��.
*
* @retval true  �]������.
* @retval false �]�����s.
*/
bool UniformBuffer::BufferSubData(const GLvoid* data, GLintptr offset, GLsizeiptr size)
{
  if (offset + size > this->size) {
    std::cerr << "ERROR(" << name << "): �]���͈͂��o�b�t�@�T�C�Y���z���Ă��܂�(buffer=" <<
      this->size << " offset=" << offset << " size=" << size << ")" << std::endl;
    return false;
  }
  if (offset == 0 && size == 0) {
    size = this->size;
  }
  glBindBuffer(GL_UNIFORM_BUFFER, ubo);
  glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
  return true;
}
