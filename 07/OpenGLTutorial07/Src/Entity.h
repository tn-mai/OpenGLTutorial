/**
* @file Entity.h
*/
#ifndef OPENGLTUTORIAL_SRC_ENTITY_H_INCLUDED
#define OPENGLTUTORIAL_SRC_ENTITY_H_INCLUDED
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"
#include "UniformBuffer.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include <functional>

namespace Entity {

class Buffer;

/**
* エンティティ用リンクリスト.
*/
struct Link {
  Link() : prev(this), next(this) {}
  void Insert(Link* e) {
    e->next->prev = e->prev;
    e->prev->next = e->next;
    e->prev = prev;
    e->next = this;
    prev->next = e;
    prev = e;
  }
  void Remove() {
    next->prev = prev;
    prev->next = next;
    prev = this;
    next = this;
  }

  Link* prev;
  Link* next;
};

/**
* エンティティ.
*/
class Entity : public Link
{
  friend class Buffer;

public:
  typedef std::function<void(Entity&, UniformBufferPtr&, double, const glm::mat4&, const glm::mat4&)> UpdateFunc;

  Entity() = default;
  Entity(const Entity&) = default;
  Entity& operator=(const Entity&) = default;

  glm::vec3& Position() { return position; }
  const glm::vec3& Position() const { return position; }
  glm::quat& Rotation() { return rotation; }
  const glm::quat& Rotation() const { return rotation; }
  glm::vec3& Scale() { return scale; }
  const glm::vec3& Scale() const { return scale; }

  const Mesh::MeshPtr& Mesh() const { return mesh; }
  glm::mat4 TRSMatrix() const;

  void BufferSubData(UniformBufferPtr& ubo, const GLvoid* data) {
    ubo->BufferSubData(data, uboOffset, uboSize);
  }

private:
  glm::vec3 position;
  glm::vec3 scale = glm::vec3(1, 1, 1);
  glm::quat rotation;
  Mesh::MeshPtr mesh;
  TexturePtr texture;
  Shader::ProgramPtr program;
  GLintptr uboOffset;
  GLsizeiptr uboSize;
  Buffer* parent;
  UpdateFunc updateFunc;
};

/**
* エンティティバッファ.
*/
class Buffer
{
public:
  /// イテレータ・定数イテレータ共通のクラステンプレート.
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
    Entity* operator->() { return static_cast<Entity*>(link); }
    const Entity* operator->() const { return static_cast<Entity*>(link); }
    Entity& operator*() { return *static_cast<Entity*>(link); }
    const Entity& operator*() const { return *static_cast<Entity*>(link); }
  private:
    T* link;
  };
  typedef IteratorBase<Link> Iterator; ///< イテレータ.
  typedef IteratorBase<const Link> ConstIterator; ///< 定数イテレータ.

  Buffer() = default;
  ~Buffer() = default;
  Buffer(const Buffer&) = default;
  Buffer& operator=(const Buffer&) = default;

  bool Initialize(size_t maxEntityCount, GLsizeiptr ubSizePerEntity, int bindingPoint, const char* name);
  bool AddEntity(const glm::vec3& pos, const Mesh::MeshPtr& m, const TexturePtr& t, const Shader::ProgramPtr& p, Entity::UpdateFunc func);
  void RemoveEntity(Entity* entity);
  void Update(double delta, const glm::mat4& matView, const glm::mat4& matProj);
  void Draw(const Mesh::BufferPtr& meshBuffer) const;
  Iterator Begin() { return Iterator(activeList.next); }
  Iterator End() { return Iterator(&activeList); }
  ConstIterator Begin() const { return ConstIterator(activeList.next); }
  ConstIterator End() const { return ConstIterator(&activeList); }

private:
  std::vector<Entity> buffer;
  Link freeList;
  Link activeList;
  UniformBufferPtr ubo;
};

inline Buffer::Iterator begin(Buffer& buffer) { return buffer.Begin(); }
inline Buffer::Iterator end(Buffer& buffer) { return buffer.End(); }
inline Buffer::ConstIterator begin(const Buffer& buffer) { return buffer.Begin(); }
inline Buffer::ConstIterator end(const Buffer& buffer) { return buffer.End(); }

} // namespace Entity

#endif // OPENGLTUTORIAL_SRC_ENTITY_H_INCLUDED