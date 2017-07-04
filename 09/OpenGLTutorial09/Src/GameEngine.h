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
#include "Uniform.h"
#include "GamePad.h"
#include <glm/glm.hpp>
#include <functional>
#include <random>

/**
* ゲームエンジンクラス.
*/
class GameEngine
{
public:
  /// ゲーム状態を更新する関数の型.
  typedef std::function<void(double)> UpdateFuncType;
  struct CameraData {
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;
  };

  static GameEngine& Instance();
  bool Init(int w, int h, const char* title);
  void Run();
  void UpdateFunc(const UpdateFuncType& func);
  const UpdateFuncType& UpdateFunc() const;

  bool LoadMeshFromFile(const char* filename);
  const Mesh::MeshPtr& GetMesh(const char* name);
  bool LoadTextureFromFile(const char* filename, GLenum wrapMode = GL_CLAMP_TO_EDGE);
  const TexturePtr& GetTexture(const char* filename) const;
  Entity::Entity* AddEntity(const glm::vec3& pos, const char* meshName, const char* texName, Entity::Entity::UpdateFuncType func, bool hasLight = true);
  void RemoveEntity(Entity::Entity*);
  void Light(int index, const Uniform::PointLight& light);
  const Uniform::PointLight& Light(int index) const;
  void AmbientLight(const glm::vec4& color);
  const glm::vec4& AmbientLight() const;
  void Camera(const CameraData& cam);
  const CameraData& Camera() const;
  std::mt19937& Rand();
  const GamePad& GetGamePad(int id) const;

  Entity::Buffer::Iterator BeginEntity() { return entityBuffer->Begin(); }
  Entity::Buffer::Iterator EndEntity() { return entityBuffer->End(); }

private:
  GameEngine() = default;
  ~GameEngine();
  GameEngine(const GameEngine&) = delete;
  GameEngine& operator=(const GameEngine&) = delete;
  void Update(double delta);
  void Render() const;

private:
  bool isInitialized = false;
  UpdateFuncType updateFunc;

  GLuint vbo = 0;
  GLuint ibo = 0;
  GLuint vao = 0;
  UniformBufferPtr uboLight;
  UniformBufferPtr uboPostEffect;
  Shader::ProgramPtr progTutorial;
  Shader::ProgramPtr progPostEffect;
  Shader::ProgramPtr progBloom1st;
  Shader::ProgramPtr progComposition;
  Shader::ProgramPtr progSimple;
  Shader::ProgramPtr progLensFlare;
  Shader::ProgramPtr progNonLighting;
  OffscreenBufferPtr offscreen;
  static const int bloomBufferCount = 6;
  OffscreenBufferPtr offBloom[bloomBufferCount];
  OffscreenBufferPtr offAnamorphic[2];

  std::unordered_map<std::string, TexturePtr> textureBuffer;
  Mesh::BufferPtr meshBuffer;
  Entity::BufferPtr entityBuffer;
  Uniform::LightingData lightData;
  CameraData camera;
  std::mt19937 rand;
};

void DefaultUpdateVertexData(Entity::Entity& e, void* ubo, double, const glm::mat4& matView, const glm::mat4& matProj);

#endif // GAMEENGINE_H_INCLUDED