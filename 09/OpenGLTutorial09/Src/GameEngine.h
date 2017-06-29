/**
* @file GameEngine.h
*/
#ifndef GAMEENGINE_H_INCLUDED
#define GAMEENGINE_H_INCLUDED
#include <GL/glew.h>
#include "UniformBuffer.h"
#include "OffscreenBuffer.h"
#include "Shader.h"
#include "Texture.h"
#include "Mesh.h"
#include "Entity.h"
#include <glm/glm.hpp>
#include <functional>
#include <random>

/**
* �Q�[���G���W���N���X.
*/
class GameEngine
{
public:
  /**
  * ���W�ϊ��f�[�^.
  */
  struct TransformationData
  {
    glm::mat4 matMVP;
    glm::mat4 matModel;
    glm::mat3x4 matNormal;
    glm::mat4 matTex;
  };

  /**
  * ���C�g�f�[�^(�_����).
  */
  struct PointLight
  {
    glm::vec4 position; ///< ���W(���[���h���W�n).
    glm::vec4 color; ///< ���邳.
  };

  static const int maxLightCount = 8; ///< ���C�g�̐�.

  /**
  * ���C�e�B���O�p�����[�^.
  */
  struct LightingData
  {
    glm::vec4 ambientColor; ///< ����.
    PointLight light[maxLightCount]; ///< ���C�g�̃��X�g.
  };

  /// �Q�[����Ԃ��X�V����֐��̌^.
  typedef std::function<void(double)> UpdateFunc;

  static GameEngine& Instance();
  bool Init();
  UpdateFunc SetUpdateFunc(const UpdateFunc& func);
  void Update(double delta);
  void Render() const;

  bool LoadMeshFromFile(const char* filename);
  const Mesh::MeshPtr& GetMesh(const char* name);
  bool LoadTextureFromFile(const char* filename);
  const TexturePtr& GetTexture(const char* filename) const;
  Entity::Entity* AddEntity(const glm::vec3& pos, const char* meshName, const char* texName, Entity::Entity::UpdateFuncType func);
  void RemoveEntity(Entity::Entity*);
  std::mt19937& Rand() { return rand; }
  void Light(int index, const PointLight& light) { lightData.light[index] = light; }
  const PointLight& Light(int index) const { return lightData.light[index]; }
  void AmbientLight(const glm::vec4& color) { lightData.ambientColor = color; }
  const glm::vec4& AmbientLight() const { return lightData.ambientColor; }
  void SetView(const glm::vec3& pos, const glm::vec3& at, const glm::vec3& up) {
    viewPos = pos;
    viewTarget = at;
    viewUp = up;
  }

private:
  GameEngine() = default;
  ~GameEngine();
  GameEngine(const GameEngine&) = delete;
  GameEngine& operator=(const GameEngine&) = delete;
  bool InitImpl();

private:
  Shader::ProgramPtr progTutorial;
  Shader::ProgramPtr progPostEffect;
  Shader::ProgramPtr progBloom1st;
  Shader::ProgramPtr progComposition;
  Shader::ProgramPtr progSimple;
  Shader::ProgramPtr progLensFlare;

  LightingData lightData;

  std::unordered_map<std::string, TexturePtr> textureBuffer;
  Mesh::BufferPtr meshBuffer;
  Entity::BufferPtr entityBuffer;
  std::mt19937 rand;

  OffscreenBufferPtr offscreen;
  static const int bloomBufferCount = 6;
  OffscreenBufferPtr offBloom[bloomBufferCount];
  OffscreenBufferPtr offAnamorphic[2];

  UpdateFunc updateFunc;

  glm::vec3 viewPos;
  glm::vec3 viewTarget;
  glm::vec3 viewUp;

private:
  bool isInitialized = false;
  GLuint vbo = 0;
  GLuint ibo = 0;
  GLuint vao = 0;
  UniformBufferPtr uboTrans;
  UniformBufferPtr uboLight;
  UniformBufferPtr uboPostEffect;
};

void DefaultUpdateVertexData(Entity::Entity& e, void* ubo, double, const glm::mat4& matView, const glm::mat4& matProj);

#endif // GAMEENGINE_H_INCLUDED