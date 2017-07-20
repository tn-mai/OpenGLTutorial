/**
* @file BufferObject.h
*/
#ifndef BUFFEROBJECT_H_INCLUDED
#define BUFFEROBJECT_H_INCLUDED
#include <GL/glew.h>

/**
* VBO, IBO, UBO.
*/
class BufferObject
{
public:
  BufferObject() = default;
  ~BufferObject() { Destroy(); }
  BufferObject(const BufferObject&) = delete;
  BufferObject& operator=(const BufferObject&) = delete;

  void Init(GLenum target, GLsizeiptr size, const GLvoid* data = nullptr, GLenum usage = GL_STATIC_DRAW);
  void Destroy();
  GLuint Id() const { return id; }

private:
  GLuint id = 0;
};

/**
* VAO.
*/
class VertexArrayObject
{
public:
  VertexArrayObject() = default;
  ~VertexArrayObject() { Destroy(); }
  VertexArrayObject(const VertexArrayObject&) = delete;
  VertexArrayObject& operator=(const VertexArrayObject&) = delete;

  void Init(GLuint vbo, GLuint ibo);
  void Destroy();
  void VertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, size_t offset);
  void Bind() const;
  void Unbind() const;

private:
  GLuint id = 0;
};

#endif // BUFFEROBJECT_H_INCLUDED