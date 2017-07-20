/**
* @file BufferObject.cpp
*/
#include "BufferObject.h"

/**
* バッファオブジェクトを作成する.
*
* @param target バッファオブジェクトの種類.
* @param size   頂点データのサイズ.
* @param data   頂点データへのポインタ.
* @param usage  バッファオブジェクトのアクセスタイプ.
*/
void BufferObject::Init(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage)
{
  Destroy();
  glGenBuffers(1, &id);
  glBindBuffer(target, id);
  glBufferData(target, size, data, usage);
  glBindBuffer(target, 0);
}

/**
* Buffer Objectを破棄する.
*/
void BufferObject::Destroy()
{
  if (id) {
    glDeleteBuffers(1, &id);
    id = 0;
  }
}

/**
* VAOを作成する.
*
* @param vbo  頂点バッファオブジェクトのID.
* @param ibo  インデックスバッファオブジェクトのID.
*/
void VertexArrayObject::Init(GLuint vbo, GLuint ibo)
{
  Destroy();
  glGenVertexArrays(1, &id);
  glBindVertexArray(id);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBindVertexArray(0);
}

/**
* VAOを破棄する.
*/
void VertexArrayObject::Destroy()
{
  if (id) {
    glDeleteVertexArrays(1, &id);
    id = 0;
  }
}

/**
* 頂点アトリビュートを設定する.
*
* @param index      頂点アトリビュートのインデックス.
* @param size       頂点アトリビュートの要素数.
* @param type       頂点アトリビュートの型.
* @param normalized GL_TRUE=要素を正規化する. GL_FALSE=正規化しない.
* @param stride     次の頂点データまでのバイト数.
* @param offset     頂点データ先頭からのバイトオフセット.
*/
void VertexArrayObject::VertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, size_t offset)
{
  glEnableVertexAttribArray(index);
  glVertexAttribPointer(index, size, type, normalized, stride, reinterpret_cast<GLvoid*>(offset));
}

/**
* VAOを処理対象にする.
*/
void VertexArrayObject::Bind() const
{
  glBindVertexArray(id);
}

/**
* VAOを処理対象から外す.
*/
void VertexArrayObject::Unbind() const
{
  glBindVertexArray(0);
}
