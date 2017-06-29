/**
* @file Entity.cpp
*/
#include "Entity.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace Entity {

/**
* �����N�I�u�W�F�N�g�������̎�O�ɒǉ�����.
*
* @param p�@�ǉ����郊���N�I�u�W�F�N�g�ւ̃|�C���^.
*
* p���������̃����N���X�g����؂藣���A�����̎�O�ɒǉ�����.
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
* �������g�������N���X�g����؂藣��.
*
* �����͂ǂ��ɂ��ڑ�����Ă��Ȃ���ԂɂȂ�.
*/
void Buffer::Link::Remove()
{
  next->prev = prev;
  prev->next = next;
  prev = this;
  next = this;
}

/**
* �g�k�E��]�E�ړ��s����擾����.
*
* @return TRS�s��.
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
* �G���e�B�e�B�o�b�t�@���쐬����.
*
* @param maxEntityCount   ������G���e�B�e�B�̍ő吔.
* @param ubSizePerEntity  �G���e�B�e�B���Ƃ�Uniform Buffer�̃o�C�g��.
* @param bindingPoint     �G���e�B�e�B�pUBO�̃o�C���f�B���O�|�C���g.
* @param ubName           �G���e�B�e�B�pUniform Buffer�̖��O.
*
* @return �쐬�����G���e�B�e�B�o�b�t�@�ւ̃|�C���^.
*/
BufferPtr Buffer::Create(size_t maxEntityCount, GLsizeiptr ubSizePerEntity, int bindingPoint, const char* ubName)
{
  struct Impl : Buffer { Impl() {} ~Impl() {} };
  BufferPtr p = std::make_shared<Impl>();
  if (!p) {
    std::cerr << "WARNING in Entity::Buffer::Create: �o�b�t�@�̍쐬�Ɏ��s." << std::endl;
    return {};
  }
  p->ubo = UniformBuffer::Create(maxEntityCount * ubSizePerEntity, bindingPoint, ubName);
  p->buffer.reset(new LinkEntity[maxEntityCount]);
  if (!p->ubo || !p->buffer) {
    std::cerr << "WARNING in Entity::Buffer::Create: �o�b�t�@�̍쐬�Ɏ��s." << std::endl;
    return {};
  }
  p->bufferSize = maxEntityCount;
  p->ubSizePerEntity = ubSizePerEntity;
  GLintptr offset = 0;
  const LinkEntity* const end = &p->buffer[maxEntityCount];
  for (LinkEntity* itr = &p->buffer[0]; itr != end; ++itr) {
    itr->uboOffset = offset;
    p->freeList.Insert(itr);
    offset += ubSizePerEntity;
  }
  return p;
}

/**
* �G���e�B�e�B��ǉ�����.
*
* @param position �G���e�B�e�B�̍��W.
* @param mesh     �G���e�B�e�B�̕\���Ɏg�p���郁�b�V��.
* @param texture  �G���e�B�e�B�̕\���Ɏg���e�N�X�`��.
* @param program  �G���e�B�e�B�̕\���Ɏg�p����V�F�[�_�v���O����.
* @param func     �G���e�B�e�B�̏�Ԃ��X�V����֐�(�܂��͊֐��I�u�W�F�N�g).
*
* @return �ǉ������G���e�B�e�B�ւ̃|�C���^.
*         ����ȏ�G���e�B�e�B��ǉ��ł��Ȃ��ꍇ��nullptr���Ԃ����.
*         ��]��g�嗦�͂��̃|�C���^�o�R�Őݒ肷��.
*         �Ȃ��A���̃|�C���^���A�v���P�[�V�������ŕێ�����K�v�͂Ȃ�.
*/
Entity* Buffer::AddEntity(const glm::vec3& position, const Mesh::MeshPtr& mesh, const TexturePtr& texture, const Shader::ProgramPtr& program, Entity::UpdateFuncType func)
{
  if (freeList.prev == freeList.next) {
    std::cerr << "WARNING in Entity::Buffer::AddEntity: �󂫃G���e�B�e�B������܂���." << std::endl;
    return nullptr;
  }
  LinkEntity* entity = static_cast<LinkEntity*>(freeList.prev);
  activeList.Insert(entity);
  entity->position = position;
  entity->rotation = glm::quat();
  entity->scale = glm::vec3(1, 1, 1);
  entity->velocity = glm::vec3();
  entity->id = -1;
  entity->color = glm::vec4(1, 1, 1, 1);
  entity->mesh = mesh;
  entity->texture = texture;
  entity->program = program;
  entity->updateFunc = func;
  entity->isActive = true;
  return entity;
}

/**
*�@�G���e�B�e�B���폜����.
*
* @param �폜����G���e�B�e�B�̃|�C���^.
*/
void Buffer::RemoveEntity(Entity* entity)
{
  if (!entity || !entity->isActive) {
    std::cerr << "WARNING in Entity::Buffer::RemoveEntity: ��A�N�e�B�u�ȃG���e�B�e�B���폜���悤�Ƃ��܂���." << std::endl;
    return;
  }
  LinkEntity* p = static_cast<LinkEntity*>(entity);
  if (p < &buffer[0] || p >= &buffer[bufferSize]) {
    std::cerr << "WARNING in Entity::Buffer::RemoveEntity: �قȂ�o�b�t�@����擾�����G���e�B�e�B���폜���悤�Ƃ��܂���." << std::endl;
    return;
  }
  if (p == itrUpdate) {
    itrUpdate = p->prev;
  }
  freeList.Insert(p);
  p->mesh.reset();
  p->texture.reset();
  p->program.reset();
  p->updateFunc = nullptr;
  p->isActive = false;
}

/**
* �A�N�e�B�u�ȃG���e�B�e�B�̏�Ԃ��X�V����.
*
* @param delta   �O��̍X�V����̌o�ߎ���.
* @param matView View�s��.
* @param matProj Projection�s��.
*/
void Buffer::Update(double delta, const glm::mat4& matView, const glm::mat4& matProj)
{
  uint8_t* p = static_cast<uint8_t*>(ubo->MapBuffer());
  for (itrUpdate = activeList.next; itrUpdate != &activeList; itrUpdate = itrUpdate->next) {
    LinkEntity& e = *static_cast<LinkEntity*>(itrUpdate);
    e.position += e.velocity * static_cast<float>(delta);
    if (e.updateFunc) {
      e.updateFunc(e, p + e.uboOffset, delta, matView, matProj);
    }
  }
  itrUpdate = nullptr;
  ubo->UnmapBuffer();
}

/**
* �A�N�e�B�u�ȃG���e�B�e�B��`�悷��.
*
* @param meshBuffer �`��Ɏg�p���郁�b�V���o�b�t�@�ւ̃|�C���^.
*/
void Buffer::Draw(const Mesh::BufferPtr& meshBuffer) const
{
  meshBuffer->BindVAO();
  for (const Link* itr = activeList.next; itr != &activeList; itr = itr->next) {
    const LinkEntity& e = *static_cast<const LinkEntity*>(itr);
    if (e.mesh && e.texture && e.program) {
      e.program->UseProgram();
      e.program->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, e.texture->Id());
      ubo->BindBufferRange(e.uboOffset, ubSizePerEntity);
      e.mesh->Draw(meshBuffer);
    }
  }
}

} // namespace Entity
