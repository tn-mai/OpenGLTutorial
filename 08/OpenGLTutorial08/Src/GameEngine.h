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

const int maxLightCount = 8; ///< ���C�g�̐�.

/**
* ���C�e�B���O�p�����[�^.
*/
struct LightingData
{
  glm::vec4 ambientColor; ///< ����.
  PointLight light[maxLightCount]; ///< ���C�g�̃��X�g.
};

/**
*
*/
class Game
{
public:
  typedef std::function<void(double)> UpdateFunc;

  static Game& Instance() {
    static Game instance;
    return instance;
  }

  bool Init();
  UpdateFunc SetUpdateFunc(const UpdateFunc& func);
  void Update(double delta);
  void Render() const;

private:
  Game() = default;
  ~Game();
  Game(const Game&) = delete;
  Game& operator=(const Game&) = delete;
  bool InitImpl();

private:
  bool isInitialized = false;
  GLuint vbo = 0;
  GLuint ibo = 0;
  GLuint vao = 0;
  UniformBufferPtr uboTrans;
  UniformBufferPtr uboLight;
  UniformBufferPtr uboPostEffect;

public:
  Shader::ProgramPtr shaderProgram;
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
};

#endif // GAMEENGINE_H_INCLUDED