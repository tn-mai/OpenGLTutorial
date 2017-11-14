/**
* @file GameEngine.h
*/
#ifndef GAMEENGINE_H_INCLUDED
#define GAMEENGINE_H_INCLUDED
#include <GL/glew.h>
#include "UniformBuffer.h"
#include "OffscreenBuffer.h"
#include "BufferObject.h"
#include "Shader.h"
#include "Texture.h"
#include "Mesh.h"
#include "Entity.h"
#include "Uniform.h"
#include "GamePad.h"
#include "Font.h"
#include <glm/glm.hpp>
#include <unordered_map>
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
  Entity::Entity* AddEntity(int groupId, const glm::vec3& pos, const char* meshName, const char* texName, Entity::Entity::UpdateFuncType func, const char* shader = nullptr);
  Entity::Entity* AddEntity(int groupId, const glm::vec3& pos, const char* meshName, const char* texName, const char* normalName, Entity::Entity::UpdateFuncType func, const char* shader = nullptr);
  void RemoveEntity(Entity::Entity*);
  void RemoveAllEntity();
  void Light(int index, const Uniform::PointLight& light);
  const Uniform::PointLight& Light(int index) const;
  void AmbientLight(const glm::vec4& color);
  const glm::vec4& AmbientLight() const;
  float KeyValue() const { return keyValue; }
  void KeyValue(float k) { keyValue = k; }
  void Camera(const CameraData& cam);
  const CameraData& Camera() const;
  std::mt19937& Rand();
  const GamePad& GetGamePad(int id) const;

  bool InitAudio(const char* acfPath, const char* acbPath, const char* awbPath, const char* dspBusName);
  void PlayAudio(int playerId, int cueId);
  void StopAudio(int playerId);

  void PushLevel();
  void PopLevel();
  void ClearLevel();

  void CollisionHandler(int gid0, int gid1, Entity::CollisionHandlerType handler);
  const Entity::CollisionHandlerType& CollisionHandler(int gid0, int gid1) const;
  void ClearCollisionHandlerList();

  bool LoadFontFromFile(const char* filename) { return fontRenderer.LoadFromFile(filename); }
  bool AddString(const glm::vec2& pos, const char* str) {return fontRenderer.AddString(pos, str); }
  void FontScale(const glm::vec2& scale) { fontRenderer.Scale(scale); }
  void FontColor(const glm::vec4& color) { fontRenderer.Color(color); }
  void FontSubColor(const glm::vec4& color) { fontRenderer.SubColor(color); }
  void FontBorder(float border) { fontRenderer.Border(border); }
  void FontThickness(float t) { fontRenderer.Thickness(t); }
  void FontPropotional(bool b) { fontRenderer.Propotional(b); }
  void FontXAdvance(float x) { fontRenderer.XAdvance(x); }

  double& UserVariable(const char* name) { return userNumbers[name]; }
  double UserVariable(const char* name) const {
    auto itr = userNumbers.find(name);
    return itr != userNumbers.end() ? itr->second : 0;
  }

  double Fps() const { return fps; }

  Entity::Buffer::Iterator BeginEntity() { return entityBuffer->Begin(); }
  Entity::Buffer::Iterator EndEntity() { return entityBuffer->End(); }

  mutable float luminanceScale = 1.0f;

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
  BufferObject pbo[2];
  int pboIndexForWriting = -1;
  float keyValue = 0.18f;

  UniformBufferPtr uboLight;
  UniformBufferPtr uboPostEffect;
  std::unordered_map<std::string, Shader::ProgramPtr> shaderMap;
  OffscreenBufferPtr offscreen;
  static const int bloomBufferCount = 4;
  OffscreenBufferPtr offBloom[bloomBufferCount];
  OffscreenBufferPtr offAnamorphic[2];

  Mesh::BufferPtr meshBuffer;

  typedef std::unordered_map<std::string, TexturePtr> TextureMap;
  static const size_t minimalStackSize = 1;
  std::vector<TextureMap> textureMapStack;

  Entity::BufferPtr entityBuffer;
  Font::Renderer fontRenderer;
  Uniform::LightingData lightData;
  CameraData camera;
  std::mt19937 rand;
  double fps = 0;

  std::unordered_map<std::string, double> userNumbers;
};

#endif // GAMEENGINE_H_INCLUDED