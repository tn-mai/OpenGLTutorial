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
typedef std::shared_ptr<Buffer> BufferPtr; ///< エンティティバッファポインタ型.

/**
* エンティティ.
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
  glm::vec3 position; ///< 座標.
  glm::quat rotation; ///< 回転.
  glm::vec3 scale = glm::vec3(1, 1, 1); ///< 大きさ.
  glm::vec3 velocity; ///< 速度.
  Mesh::MeshPtr mesh; ///< エンティティを描画するときに使われるメッシュデータ.
  TexturePtr texture; ///< エンティティを描画するときに使われるテクスチャ.
  Shader::ProgramPtr program; ///< エンティティを描画するときに使われるシェーダ.
  GLintptr uboOffset; ///< UBOのエンティティ用領域へのバイトオフセット.
  Buffer* parent; ///< 作成元へのポインタ.
  UpdateFuncType updateFunc; ///< 状態更新関数.
  bool isActive = false;
};

/**
* エンティティバッファ.
*/
class Buffer
{
  /// エンティティ用リンクリスト.
  struct Link {
    void Insert(Link* e);
    void Remove();
    Link* prev = this;
    Link* next = this;
  };
  /// リンクつきエンティティ.
  struct LinkEntity : public Link, public Entity {};

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
    Entity* operator->() { return static_cast<LinkEntity*>(link); }
    const Entity* operator->() const { return static_cast<LinkEntity*>(link); }
    Entity& operator*() { return *static_cast<LinkEntity*>(link); }
    const Entity& operator*() const { return *static_cast<LinkEntity*>(link); }
  private:
    T* link;
  };
  typedef IteratorBase<Link> Iterator; ///< イテレータ.
  typedef IteratorBase<const Link> ConstIterator; ///< 定数イテレータ.

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