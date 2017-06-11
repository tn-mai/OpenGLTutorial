/**
* @file Mesh.h
*/
#ifndef OPENGLTUTORIAL_SRC_MESH_H_INCLUDED
#define OPENGLTUTORIAL_SRC_MESH_H_INCLUDED
#include <GL/glew.h>
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

namespace Mesh {

typedef int MeshId;
static const MeshId INVALID_MESH_ID = -1;

/**
* ���b�V���f�[�^.
*/
class Mesh
{
public:
  Mesh() = default;
  Mesh(const std::string& n, GLenum t, GLsizei s, GLsizeiptr off);
  Mesh(const Mesh&) = default;
  Mesh& operator=(const Mesh&) = default;

  const std::string& Name() const { return name; }
  GLenum Mode() const { return GL_TRIANGLES; } 
  GLsizei Size() const { return size; }
  GLvoid* Offset() const { return offset; }
  GLenum Type() const { return type; }

private:
  std::string name; ///< ���b�V���f�[�^��.
  std::vector<std::string> textureList; ///< �e�N�X�`�����̃��X�g.
  GLenum type = GL_UNSIGNED_SHORT; ///< �C���f�b�N�X�̃f�[�^�^.
  GLsizei size = 0; ///< �`�悷��C���f�b�N�X��.
  GLvoid* offset = nullptr; ///< �`��J�n�C���f�b�N�X�̃o�C�g�I�t�Z�b�g.
};

class Buffer;
typedef std::shared_ptr<Buffer> BufferPtr;
typedef std::shared_ptr<Mesh> MeshPtr;

/**
* ���b�V���f�[�^�o�b�t�@.
*/
class Buffer
{
public:
  static BufferPtr Create(int vboSize, int iboSize);
  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;

  bool LoadMeshFromFile(const char* filename);
  const MeshPtr GetMesh(const char* name) const;

  void BindVAO() const {
    glBindVertexArray(vao);
  }

  void Draw(const MeshPtr&) const;

private:
  Buffer() = default;
  ~Buffer();

private:
  GLuint vbo = 0;
  GLuint ibo = 0;
  GLuint vao = 0;
  GLintptr vboEnd = 0;
  GLintptr iboEnd = 0;

  std::unordered_map<std::string, MeshPtr> meshList;
};

} // namespace Mesh

#endif // OPENGLTUTORIAL_SRC_MESH_H_INCLUDED