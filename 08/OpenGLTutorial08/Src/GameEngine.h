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
    glm::mat4 matM;
    glm::mat4 matMVP;
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

private:
  GameEngine() = default;
  ~GameEngine();
  GameEngine(const GameEngine&) = delete;
  GameEngine& operator=(const GameEngine&) = delete;
  bool InitImpl();

public:
  Shader::ProgramPtr progTutorial;
  Shader::ProgramPtr progPostEffect;
  Shader::ProgramPtr progBloom1st;
  Shader::ProgramPtr progComposition;
  Shader::ProgramPtr progSimple;
  Shader::ProgramPtr progLensFlare;

  LightingData lightData;

  TexturePtr tex;
  TexturePtr texSample;
  Mesh::BufferPtr meshBuffer;
  Mesh::MeshPtr sampleMesh[2];

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

#endif // GAMEENGINE_H_INCLUDED