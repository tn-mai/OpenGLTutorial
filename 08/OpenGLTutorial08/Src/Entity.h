/**
* @file Entity.h
*/
#ifndef OPENGLTUTORIAL_SRC_ENTITY_H_INCLUDED
#define OPENGLTUTORIAL_SRC_ENTITY_H_INCLUDED
#include <GL/glew.h>
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"
#include "UniformBuffer.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>
#include <functional>

namespace Entity {

class Buffer;
typedef std::shared_ptr<Buffer> BufferPtr; ///< �G���e�B�e�B�o�b�t�@�|�C���^�^.

/**
* �G���e�B�e�B.
*/
class Entity
{
  friend class Buffer;

public:
  typedef std::function<void(Entity&, void*, double, const glm::mat4&, const glm::mat4&)> UpdateFuncType;

  void Position(const glm::vec3& pos) { position = pos; }
  const glm::vec3& Position() const { return position; }
  void Rotation(const glm::quat& rot) { rotation = rot; }
  const glm::quat& Rotation() const { return rotation; }
  void Scale(const glm::vec3& s) { scale = s; }
  const glm::vec3& Scale() const { return scale; }
  void Velocity(const glm::vec3& v) { velocity = v; }
  const glm::vec3& Velocity() const { return velocity; }
  void UpdateFunc(const UpdateFuncType& func) { updateFunc = func; }
  const UpdateFuncType& UpdateFunc() const { return updateFunc; }

  const Mesh::MeshPtr& Mesh() const { return mesh; }
  const TexturePtr& Texture() const { return texture; }
  const Shader::ProgramPtr& ShaderProgram() const { return program; }
  glm::mat4 TRSMatrix() const;
  Buffer* Parent() { return parent; }

private:
  Entity() = default;
  ~Entity() = default;
  Entity(const Entity&) = default;
  Entity& operator=(const Entity&) = default;

private:
  glm::vec3 position; ///< ���W.
  glm::quat rotation; ///< ��].
  glm::vec3 scale = glm::vec3(1, 1, 1); ///< �傫��.
  glm::vec3 velocity; ///< ���x.
  Mesh::MeshPtr mesh; ///< �G���e�B�e�B��`�悷��Ƃ��Ɏg���郁�b�V���f�[�^.
  TexturePtr texture; ///< �G���e�B�e�B��`�悷��Ƃ��Ɏg����e�N�X�`��.
  Shader::ProgramPtr program; ///< �G���e�B�e�B��`�悷��Ƃ��Ɏg����V�F�[�_.
  GLintptr uboOffset; ///< UBO�̃G���e�B�e�B�p�̈�ւ̃o�C�g�I�t�Z�b�g.
  Buffer* parent; ///< �쐬���ւ̃|�C���^.
  UpdateFuncType updateFunc; ///< ��ԍX�V�֐�.
  bool isActive = false;
};

/**
* �G���e�B�e�B�o�b�t�@.
*/
class Buffer
{
  /// �G���e�B�e�B�p�����N���X�g.
  struct Link {
    void Insert(Link* e);
    void Remove();
    Link* prev = this;
    Link* next = this;
  };
  /// �����N���G���e�B�e�B.
  struct LinkEntity : public Link, public Entity {};

public:
  /// �C�e���[�^�E�萔�C�e���[�^���ʂ̃N���X�e���v���[�g.
  template<typename T>
  class IteratorBase : public std::iterator<std::bidirectional_iterator_tag, Entity>
  {
  public:
    IteratorBase() = default;
    explicit IteratorBase(T* p) : link(p) {}

    explicit operator bool() const { return link; }
    bool operator!() const { return !link; }
    bool operator==(const IteratorBase& rhs) const { return link == rhs.link; }
    bool operator!=(const IteratorBase& rhs) const { return link != rhs.link; }
    friend void swap(IteratorBase& lhs, IteratorBase& rhs) {
      T tmp = lhs.link;
      lhs.link = rhs.link;
      rhs.link = tmp;
    }

    IteratorBase& operator++() { link = link->next; return *this; }
    IteratorBase operator++(int) { IteratorBase tmp = *this;  link = link->next; return tmp; }
    IteratorBase& operator--() { link = link->prev; return *this; }
    IteratorBase operator--(int) { IteratorBase tmp = *this;  link = link->prev; return tmp; }
    Entity* operator->() { return static_cast<LinkEntity*>(link); }
    const Entity* operator->() const { return static_cast<LinkEntity*>(link); }
    Entity& operator*() { return *static_cast<LinkEntity*>(link); }
    const Entity& operator*() const { return *static_cast<LinkEntity*>(link); }
  private:
    T* link;
  };
  typedef IteratorBase<Link> Iterator; ///< �C�e���[�^.
  typedef IteratorBase<const Link> ConstIterator; ///< �萔�C�e���[�^.

  static BufferPtr Create(size_t maxEntityCount, GLsizeiptr ubSizePerEntity, int bindingPoint, const char* ubName);

  Entity* AddEntity(const glm::vec3& pos, const Mesh::MeshPtr& m, const TexturePtr& t, const Shader::ProgramPtr& p, Entity::UpdateFuncType func);
  void RemoveEntity(Entity* entity);
  void Update(double delta, const glm::mat4& matView, const glm::mat4& matProj);
  void Draw(const Mesh::BufferPtr& meshBuffer) const;
  Iterator Begin() { return Iterator(activeList.next); }
  Iterator End() { return Iterator(&activeList); }
  ConstIterator Begin() const { return ConstIterator(activeList.next); }
  ConstIterator End() const { return ConstIterator(&activeList); }

private:
  Buffer() = default;
  ~Buffer() = default;
  Buffer(const Buffer&) = default;
  Buffer& operator=(const Buffer&) = default;

private:
  struct EntityArrayDeleter { void operator()(LinkEntity* p) { delete[] p; } };

  std::unique_ptr<LinkEntity[], EntityArrayDeleter> buffer;
  size_t bufferSize;
  GLsizeiptr ubSizePerEntity;
  Link freeList;
  Link activeList;
  UniformBufferPtr ubo;
  Link* itrUpdate = nullptr;
};

inline Buffer::Iterator begin(Buffer& buffer) { return buffer.Begin(); }
inline Buffer::Iterator end(Buffer& buffer) { return buffer.End(); }
inline Buffer::ConstIterator begin(const Buffer& buffer) { return buffer.Begin(); }
inline Buffer::ConstIterator end(const Buffer& buffer) { return buffer.End(); }

} // namespace Entity

#endif // OPENGLTUTORIAL_SRC_ENTITY_H_INCLUDED