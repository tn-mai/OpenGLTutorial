/**
* @file Entity.h
*/
#ifndef ENTITY_H_INCLUDED
#define ENTITY_H_INCLUDED
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
using BufferPtr = std::shared_ptr<Buffer>; ///< �G���e�B�e�B�o�b�t�@�|�C���^�^.

/**
* �G���e�B�e�B.
*/
class Entity
{
  friend class Buffer;

public:
  /// ��ԍX�V�֐��^.
  using UpdateFuncType = std::function<void(Entity&, void*, double, const glm::mat4&, const glm::mat4&)>;

  void Position(const glm::vec3& v) { position = v; }
  const glm::vec3& Position() const { return position; }
  void Rotation(const glm::quat& q) { rotation = q; }
  const glm::quat& Rotation() const { return rotation; }
  void Scale(const glm::vec3& v) { scale = v; }
  const glm::vec3& Scale() const { return scale; }
  void Color(const glm::vec4& v) { color = v; }
  const glm::vec4& Color() const { return color; }
  void Velocity(const glm::vec3& v) { velocity = v; }
  const glm::vec3& Velocity() const { return velocity; }
  void UpdateFunc(const UpdateFuncType& func) { updateFunc = func; }
  const UpdateFuncType& UpdateFunc() const { return updateFunc; }

  glm::mat4 CalcModelMatrix() const;

private:
  Entity() = default;
  ~Entity() = default;
  Entity(const Entity&) = default;
  Entity& operator=(const Entity&) = default;

private:
  glm::vec3 position; ///< ���W.
  glm::vec3 scale = glm::vec3(1, 1, 1); ///< �g�嗦.
  glm::quat rotation; ///< ��].
  glm::vec4 color = glm::vec4(1, 1, 1, 1); ///< �F.
  glm::vec3 velocity; ///< ���x.
  UpdateFuncType updateFunc; ///< ��ԍX�V�֐�.
  Mesh::MeshPtr mesh; ///< �G���e�B�e�B��`�悷��Ƃ��Ɏg���郁�b�V���f�[�^.
  TexturePtr texture; ///< �G���e�B�e�B��`�悷��Ƃ��Ɏg����e�N�X�`��.
  Shader::ProgramPtr program; ///< �G���e�B�e�B��`�悷��Ƃ��Ɏg����V�F�[�_.
  GLintptr uboOffset; ///< UBO�̃G���e�B�e�B�p�̈�ւ̃o�C�g�I�t�Z�b�g.
  bool isActive = false; ///< �A�N�e�B�u�ȃG���e�B�e�B�Ȃ�true, ��A�N�e�B�u�Ȃ�false.
};

/**
* �G���e�B�e�B�o�b�t�@.
*/
class Buffer
{
public:
  static BufferPtr Create(size_t maxEntityCount, GLsizeiptr ubSizePerEntity,
    int bindingPoint, const char* name);

  Entity* AddEntity(const glm::vec3& pos, const Mesh::MeshPtr& m, const TexturePtr& t,
    const Shader::ProgramPtr& p, Entity::UpdateFuncType func);
  void RemoveEntity(Entity* entity);
  void Update(double delta, const glm::mat4& matView, const glm::mat4& matProj);
  void Draw(const Mesh::BufferPtr& meshBuffer) const;
  const UniformBufferPtr& UniformBuffer() const { return ubo; }

private:
  Buffer() = default;
  ~Buffer() = default;
  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;

private:
  /// �G���e�B�e�B�p�����N���X�g.
  struct Link {
    void Insert(Link* e);
    void Remove();
    Link* prev = this;
    Link* next = this;
  };
  /// �����N���G���e�B�e�B.
  struct LinkEntity : public Link, public Entity {};
  /// �����N�t���G���e�B�e�B�z��̍폜�p�֐��I�u�W�F�N�g.
  struct EntityArrayDeleter { void operator()(LinkEntity* p) { delete[] p; } };

  std::unique_ptr<LinkEntity[], EntityArrayDeleter> buffer; ///< �G���e�B�e�B�̔z��.
  size_t bufferSize; ///< �G���e�B�e�B�̑���.
  Link freeList; ///< ���g�p�̃G���e�B�e�B�̃����N���X�g.
  Link activeList; ///< �g�p���̃G���e�B�e�B�̃����N���X�g.
  GLsizeiptr ubSizePerEntity; ///< �e�G���e�B�e�B���g����Uniform Buffer�̃o�C�g��.
  UniformBufferPtr ubo; ///< �G���e�B�e�B�pUBO.
  Link* itrUpdate = nullptr; ///< Update��RemoveEntity�̑��ݍ�p�ɑΉ����邽�߂̃C�e���[�^.
};

} // namespace Entity

#endif