/**
* @file Entity.cpp
*/
#include "Entity.h"
#include "Uniform.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>

namespace Entity {

/**
* VertexDataをUBOに転送する.
*
* @param entity
* @param ubo
* @param matViewProjection
*/
void UpdateUniformVertexData(Entity& entity, void* ubo, const glm::mat4& matViewProjection)
{
  Uniform::VertexData data;
  data.matModel = entity.TRSMatrix();
  data.matNormal = glm::mat4_cast(entity.Rotation());
  data.matMVP = matViewProjection * data.matModel;
  data.color = entity.Color();
  memcpy(ubo, &data, sizeof(data));
}

/**
* リンクオブジェクトを自分の手前に追加する.
*
* @param p　追加するリンクオブジェクトへのポインタ.
*
* pを所属元のリンクリストから切り離し、自分の手前に追加する.
*/
void Buffer::Link::Insert(Link* p)
{
  p->Remove();
  p->prev = prev;
  p->next = this;
  prev->next = p;
  prev = p;
}

/**
* 自分自身をリンクリストから切り離す.
*
* 自分はどこにも接続されていない状態になる.
*/
void Buffer::Link::Remove()
{
  next->prev = prev;
  prev->next = next;
  prev = this;
  next = this;
}

/**
* 拡縮・回転・移動行列を取得する.
*
* @return TRS行列.
*/
glm::mat4 Entity::TRSMatrix() const
{
#if 1
  return glm::scale(glm::translate(glm::mat4(), position) * glm::mat4_cast(rotation), scale);
#else
  const glm::mat4 s = glm::scale(glm::mat4(), scale);
  const glm::mat4 r = glm::mat4_cast(rotation);
  const glm::mat4 t = glm::translate(glm::mat4(), position);
  return t * r * s;
#endif
}

/**
* エンティティを破棄する.
*
* この関数を呼び出したあとは、エンティティを操作してはならない.
*/
void Entity::Destroy()
{
  if (pBuffer) {
    pBuffer->RemoveEntity(this);
  }
}

/**
* エンティティバッファを作成する.
*
* @param maxEntityCount   扱えるエンティティの最大数.
* @param ubSizePerEntity  エンティティごとのUniform Bufferのバイト数.
* @param bindingPoint     エンティティ用UBOのバインディングポイント.
* @param ubName           エンティティ用Uniform Bufferの名前.
*
* @return 作成したエンティティバッファへのポインタ.
*/
BufferPtr Buffer::Create(size_t maxEntityCount, GLsizeiptr ubSizePerEntity, int bindingPoint, const char* ubName)
{
  struct Impl : Buffer { Impl() {} ~Impl() {} };
  BufferPtr p = std::make_shared<Impl>();
  if (!p) {
    std::cerr << "WARNING in Entity::Buffer::Create: バッファの作成に失敗." << std::endl;
    return {};
  }
  p->ubo = UniformBuffer::Create(maxEntityCount * ubSizePerEntity, bindingPoint, ubName);
  p->buffer.reset(new LinkEntity[maxEntityCount]);
  if (!p->ubo || !p->buffer) {
    std::cerr << "WARNING in Entity::Buffer::Create: バッファの作成に失敗." << std::endl;
    return {};
  }
  p->bufferSize = maxEntityCount;
  p->ubSizePerEntity = ubSizePerEntity;
  GLintptr offset = 0;
  const LinkEntity* const end = &p->buffer[maxEntityCount];
  for (LinkEntity* itr = &p->buffer[0]; itr != end; ++itr) {
    itr->uboOffset = offset;
    itr->pBuffer = p.get();
    p->freeList.Insert(itr);
    offset += ubSizePerEntity;
  }
  p->collisionHandlerList.reserve(maxGroupId);
  return p;
}

/**
* エンティティを追加する.
*
* @param groupId  エンティティのグループID.
* @param position エンティティの座標.
* @param mesh     エンティティの表示に使用するメッシュ.
* @param texture  エンティティの表示に使うテクスチャ.
* @param program  エンティティの表示に使用するシェーダプログラム.
* @param func     エンティティの状態を更新する関数(または関数オブジェクト).
*
* @return 追加したエンティティへのポインタ.
*         これ以上エンティティを追加できない場合はnullptrが返される.
*         回転や拡大率はこのポインタ経由で設定する.
*         なお、このポインタをアプリケーション側で保持する必要はない.
*/
Entity* Buffer::AddEntity(int groupId, const glm::vec3& position, const Mesh::MeshPtr& mesh, const TexturePtr t[2], const Shader::ProgramPtr& program, const Entity::UpdateFuncType& func)
{
  if (freeList.prev == freeList.next) {
    std::cerr << "WARNING in Entity::Buffer::AddEntity: 空きエンティティがありません." << std::endl;
    return nullptr;
  }
  if (groupId < 0 || groupId > maxGroupId) {
    std::cerr << "ERROR in Entity::Buffer::AddEntity: 範囲外のグループID(" << groupId << ")が渡されました.\nグループIDは0～" << maxGroupId << "でなければなりません." << std::endl;
    return nullptr;
  }
  LinkEntity* entity = static_cast<LinkEntity*>(freeList.prev);
  activeList[groupId].Insert(entity);
  entity->groupId = groupId;
  entity->position = position;
  entity->rotation = glm::quat();
  entity->scale = glm::vec3(1, 1, 1);
  entity->velocity = glm::vec3();
  entity->color = glm::vec4(1, 1, 1, 1);
  entity->mesh = mesh;
  entity->texture[0] = t[0];
  entity->texture[1] = t[1];
  entity->program = program;
  entity->updateFunc = func;
  entity->isActive = true;
  return entity;
}

/**
*　エンティティを削除する.
*
* @param 削除するエンティティのポインタ.
*/
void Buffer::RemoveEntity(Entity* entity)
{
  if (!entity || !entity->isActive) {
    std::cerr << "WARNING in Entity::Buffer::RemoveEntity: 非アクティブなエンティティを削除しようとしました." << std::endl;
    return;
  }
  LinkEntity* p = static_cast<LinkEntity*>(entity);
  if (p < &buffer[0] || p >= &buffer[bufferSize]) {
    std::cerr << "WARNING in Entity::Buffer::RemoveEntity: 異なるバッファから取得したエンティティを削除しようとしました." << std::endl;
    return;
  }
  if (p == itrUpdate) {
    itrUpdate = p->prev;
  }
  if (p == itrUpdateRhs) {
    itrUpdateRhs = p->prev;
  }
  freeList.Insert(p);
  p->mesh.reset();
  for (auto& e : p->texture) {
    e.reset();
  }
  p->program.reset();
  p->updateFunc = nullptr;
  p->isActive = false;
}

/**
*
*/
void Buffer::RemoveAllEntity()
{
  for (int groupId = 0; groupId <= maxGroupId; ++groupId) {
    while (activeList[groupId].next != &activeList[groupId]) {
      RemoveEntity(static_cast<LinkEntity*>(activeList[groupId].next));
    }
  }
}

/**
* 矩形同士の衝突判定.
*/
bool HasCollision(const CollisionData& lhs, const CollisionData& rhs)
{
  if (lhs.max.x < rhs.min.x || lhs.min.x > rhs.max.x) return false;
  if (lhs.max.y < rhs.min.y || lhs.min.y > rhs.max.y) return false;
  if (lhs.max.z < rhs.min.z || lhs.min.z > rhs.max.z) return false;
  return true;
}

/**
* アクティブなエンティティの状態を更新する.
*
* @param delta   前回の更新からの経過時間.
* @param matView View行列.
* @param matProj Projection行列.
*/
void Buffer::Update(double delta, const glm::mat4& matView, const glm::mat4& matProj)
{
  // 座標とワールド座標系の衝突形状を更新する.
  // 各エンティティの状態を更新する.
  for (int groupId = 0; groupId <= maxGroupId; ++groupId) {
    for (itrUpdate = activeList[groupId].next; itrUpdate != &activeList[groupId]; itrUpdate = itrUpdate->next) {
      LinkEntity& e = *static_cast<LinkEntity*>(itrUpdate);
      e.position += e.velocity * static_cast<float>(delta);
      if (e.updateFunc) {
        e.updateFunc(e, delta);
      }
      e.colWorld.min = e.colLocal.min + e.position;
      e.colWorld.max = e.colLocal.max + e.position;
    }
  }

  // 衝突判定を実行する.
  for (const auto& e : collisionHandlerList) {
    if (!e.handler) {
      continue;
    }
    Link* listL = &activeList[e.groupId[0]];
    Link* listR = &activeList[e.groupId[1]];
    for (itrUpdate = listL->next; itrUpdate != listL; itrUpdate = itrUpdate->next) {
      LinkEntity* entityL = static_cast<LinkEntity*>(itrUpdate);
      for (itrUpdateRhs = listR->next; itrUpdateRhs != listR; itrUpdateRhs = itrUpdateRhs->next) {
        LinkEntity* entityR = static_cast<LinkEntity*>(itrUpdateRhs);
        if (!HasCollision(entityL->colWorld, entityR->colWorld)) {
          continue;
        }
        e.handler(*entityL, *entityR);
        if (entityL != itrUpdate) {
          break; // 左辺が削除された場合は右辺のループを終了する.
        }
      }
    }
  }
  itrUpdate = nullptr;
  itrUpdateRhs = nullptr;

  uint8_t* p = static_cast<uint8_t*>(ubo->MapBuffer());
  const glm::mat4 matVP = matProj * matView;
  for (int groupId = 0; groupId <= maxGroupId; ++groupId) {
    for (Link* itr = activeList[groupId].next; itr != &activeList[groupId]; itr = itr->next) {
      LinkEntity& e = *static_cast<LinkEntity*>(itr);
      UpdateUniformVertexData(e, p + e.uboOffset, matVP);
    }
  }
  ubo->UnmapBuffer();
}

/**
* アクティブなエンティティを描画する.
*
* @param meshBuffer 描画に使用するメッシュバッファへのポインタ.
*/
void Buffer::Draw(const Mesh::BufferPtr& meshBuffer) const
{
  meshBuffer->BindVAO();
  for (int groupId = 0; groupId <= maxGroupId; ++groupId) {
    for (const Link* itr = activeList[groupId].next; itr != &activeList[groupId]; itr = itr->next) {
      const LinkEntity& e = *static_cast<const LinkEntity*>(itr);
      if (e.mesh && e.texture && e.program) {
        e.program->UseProgram();
        for (size_t i = 0; i < sizeof(e.texture) / sizeof(e.texture[0]); ++i) {
          e.program->BindTexture(GL_TEXTURE0 + i, GL_TEXTURE_2D, e.texture[i]->Id());
        }
        ubo->BindBufferRange(e.uboOffset, ubSizePerEntity);
        e.mesh->Draw(meshBuffer);
      }
    }
  }
}

/**
* 衝突解決ハンドラを設定する.
*
* @param gid0    衝突対象のグループID.
* @param gid1    衝突対象のグループID.
* @param handler 衝突解決ハンドラ.
*
* 衝突が発生し衝突解決ハンドラが呼びされるとき、より小さいグループIDを持つエンティティから先に渡される.
* ここで指定したグループIDの順序とは無関係であることに注意すること.
* ex)
*   CollisionHandler(10, 1, Func)
*   というコードでハンドラを登録したとする. 衝突が発生すると、
*   Func(グループID=1のエンティティ、グループID=10のエンティティ)
*   のように呼び出される.
*/
void Buffer::CollisionHandler(int gid0, int gid1, const CollisionHandlerType& handler)
{
  if (gid0 > gid1) {
    std::swap(gid0, gid1);
  }
  auto itr = std::find_if(collisionHandlerList.begin(), collisionHandlerList.end(),
    [&](const CollisionHandlerElement& e) { return e.groupId[0] == gid0 && e.groupId[1] == gid1; });
  if (itr == collisionHandlerList.end()) {
    collisionHandlerList.push_back({ { gid0, gid1 }, handler });
  } else {
    itr->handler = handler;
  }
}

/**
* 衝突解決ハンドラを取得する.
*
* @param gid0 衝突対象のグループID.
* @param gid1 衝突対象のグループID.
*
* @return 衝突解決ハンドラ.
*/
const CollisionHandlerType& Buffer::CollisionHandler(int gid0, int gid1) const
{
  if (gid0 > gid1) {
    std::swap(gid0, gid1);
  }
  auto itr = std::find_if(collisionHandlerList.begin(), collisionHandlerList.end(),
    [&](const CollisionHandlerElement& e) { return e.groupId[0] == gid0 && e.groupId[1] == gid1; });
  if (itr == collisionHandlerList.end()) {
    static const CollisionHandlerType dummy;
    return dummy;
  }
  return itr->handler;
}

/**
* 全ての衝突解決ハンドラを削除する.
*/
void Buffer::ClearCollisionHandlerList()
{
  collisionHandlerList.clear();
}

} // namespace Entity
