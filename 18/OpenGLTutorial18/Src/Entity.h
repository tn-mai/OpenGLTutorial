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
#include <vector>

namespace Entity {

class Entity;
class Buffer;
typedef std::shared_ptr<Buffer> BufferPtr; ///< エンティティバッファポインタ型.
typedef std::function<void(Entity&, Entity&)> CollisionHandlerType; ///< 衝突解決ハンドラ型.

static const int maxGroupId = 15; ///< グループIDの最大値.

/**
* 衝突判定.
*/
struct CollisionData
{
  glm::vec3 min;
  glm::vec3 max;
};

/**
* エンティティ.
*/
class Entity
{
  friend class Buffer;

public:
  typedef std::function<void(Entity&, double)> UpdateFuncType;

  void Position(const glm::vec3& pos) { position = pos; }
  const glm::vec3& Position() const { return position; }
  void Rotation(const glm::quat& rot) { rotation = rot; }
  const glm::quat& Rotation() const { return rotation; }
  void Scale(const glm::vec3& s) { scale = s; }
  const glm::vec3& Scale() const { return scale; }
  void Velocity(const glm::vec3& v) { velocity = v; }
  const glm::vec3& Velocity() const { return velocity; }
  void Color(const glm::vec4& c) { color = c; }
  const glm::vec4& Color() const { return color; }
  void UpdateFunc(const UpdateFuncType& func) { updateFunc = func; }
  UpdateFuncType& UpdateFunc() { return updateFunc; }
  const UpdateFuncType& UpdateFunc() const { return updateFunc; }
  void Collision(const CollisionData& c) { colLocal = c; }
  const CollisionData& Collision() const { return colLocal; }
  void Texture(size_t n, const TexturePtr& p) { texture[n] = p; }
  const TexturePtr& Texture(size_t n) const { return texture[n]; }

  glm::mat4 TRSMatrix() const;
  int GroupId() const { return groupId; }

  void Destroy();

private:
  Entity() = default;
  ~Entity() = default;
  Entity(const Entity&) = default;
  Entity& operator=(const Entity&) = default;

private:
  int groupId = -1;
  Buffer* pBuffer = nullptr; ///< 生成元のBufferクラスへのポインタ.
  glm::vec3 position; ///< 座標.
  glm::quat rotation; ///< 回転.
  glm::vec3 scale = glm::vec3(1, 1, 1); ///< 大きさ.
  glm::vec3 velocity; ///< 速度.
  glm::vec4 color = glm::vec4(1, 1, 1, 1); ///< 色.
  Mesh::MeshPtr mesh; ///< エンティティを描画するときに使われるメッシュデータ.
  TexturePtr texture[2]; ///< エンティティを描画するときに使われるテクスチャ.
  Shader::ProgramPtr program; ///< エンティティを描画するときに使われるシェーダ.
  GLintptr uboOffset; ///< UBOのエンティティ用領域へのバイトオフセット.
  UpdateFuncType updateFunc; ///< 状態更新関数.
  CollisionData colLocal;
  CollisionData colWorld;
  bool isActive = false;
};

/**
* エンティティバッファ.
*/
class Buffer
{
  struct Link;
  struct LinkEntity;
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

  Entity* AddEntity(int groupId, const glm::vec3& pos, const Mesh::MeshPtr& m, const TexturePtr t[2], const Shader::ProgramPtr& p, const Entity::UpdateFuncType& func);
  void RemoveEntity(Entity* entity);
  void RemoveAllEntity();
  void Update(double delta, const glm::mat4& matView, const glm::mat4& matProj);
  void Draw(const Mesh::BufferPtr& meshBuffer) const;

  void CollisionHandler(int gid0, int gid1, const CollisionHandlerType& handler);
  const CollisionHandlerType& CollisionHandler(int gid0, int gid1) const;
  void ClearCollisionHandlerList();

  Iterator Begin() { return Iterator(activeList[0].next); }
  Iterator End() { return Iterator(&activeList[0]); }
  ConstIterator Begin() const { return ConstIterator(activeList[0].next); }
  ConstIterator End() const { return ConstIterator(&activeList[0]); }

private:
  Buffer() = default;
  ~Buffer() = default;
  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;

private:
  /// エンティティ用リンクリスト.
  struct Link {
    void Insert(Link* e);
    void Remove();
    Link* prev = this;
    Link* next = this;
  };
  /// リンクつきエンティティ.
  struct LinkEntity : public Link, public Entity {};
  /// リンク付きエンティティ配列の削除関数.
  struct EntityArrayDeleter { void operator()(LinkEntity* p) { delete[] p; } };

  std::unique_ptr<LinkEntity[], EntityArrayDeleter> buffer;
  size_t bufferSize;
  GLsizeiptr ubSizePerEntity;
  Link freeList;
  Link activeList[maxGroupId + 1];
  UniformBufferPtr ubo;
  Link* itrUpdate = nullptr;
  Link* itrUpdateRhs = nullptr;

  struct CollisionHandlerElement {
    int groupId[2];
    CollisionHandlerType handler;
  };
  std::vector<CollisionHandlerElement> collisionHandlerList;
};

inline Buffer::Iterator begin(Buffer& buffer) { return buffer.Begin(); }
inline Buffer::Iterator end(Buffer& buffer) { return buffer.End(); }
inline Buffer::ConstIterator begin(const Buffer& buffer) { return buffer.Begin(); }
inline Buffer::ConstIterator end(const Buffer& buffer) { return buffer.End(); }

} // namespace Entity

#endif // OPENGLTUTORIAL_SRC_ENTITY_H_INCLUDED