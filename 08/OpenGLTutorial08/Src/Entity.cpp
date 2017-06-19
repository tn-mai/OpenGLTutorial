/**
* @file Entity.cpp
*/
#include "Entity.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Entity {

/**
* ägèkÅEâÒì]ÅEà⁄ìÆçsóÒÇéÊìæÇ∑ÇÈ.
*
* @param 
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
*
*/
bool Buffer::Initialize(size_t maxEntityCount, GLsizeiptr ubSizePerEntity, int bindingPoint, const char* name)
{
  ubo = UniformBuffer::Create(maxEntityCount * ubSizePerEntity, bindingPoint, name);
  buffer.resize(maxEntityCount);
  GLintptr offset = 0;
  for (Entity& e : buffer) {
    e.uboOffset = offset;
    e.uboSize = ubSizePerEntity;
    e.parent = this;
    freeList.Insert(&e);
    offset += ubSizePerEntity;
  }
  return true;
}

/**
*
*/
bool Buffer::AddEntity(const glm::vec3& pos, const Mesh::MeshPtr& m, const TexturePtr& t, const Shader::ProgramPtr& p, Entity::UpdateFunc func)
{
  if (freeList.prev == freeList.next) {
    return false;
  }
  Entity* entity = static_cast<Entity*>(freeList.prev);
  activeList.Insert(entity);
  entity->position = pos;
  entity->mesh = m;
  entity->texture = t;
  entity->program = p;
  entity->updateFunc = func;
  return true;
}

/**
*
*/
void Buffer::RemoveEntity(Entity* entity)
{
  freeList.Insert(entity);
  entity->mesh.reset();
  entity->texture.reset();
  entity->program.reset();
}

/**
*
*/
void Buffer::Update(double delta, const glm::mat4& matView, const glm::mat4& matProj)
{
  ubo->Bind();
  for (Entity* e = static_cast<Entity*>(activeList.next); e != &activeList; e = static_cast<Entity*>(e->next)) {
    e->updateFunc(*e, ubo, delta, matView, matProj);
  }
}

/**
*
*/
void Buffer::Draw(const Mesh::BufferPtr& meshBuffer) const
{
  meshBuffer->BindVAO();
  for (const Entity* e = static_cast<const Entity*>(activeList.next); e != &activeList; e = static_cast<Entity*>(e->next)) {
    e->program->UseProgram();
    e->program->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, e->texture->Id());
    ubo->BindBufferRange(e->uboOffset, e->uboSize);
    e->Mesh()->Draw(meshBuffer);
  }
}

} // namespace Entity
