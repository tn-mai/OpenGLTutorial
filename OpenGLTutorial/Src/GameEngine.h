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
#include "InterfaceBlock.h"
#include <glm/glm.hpp>
#include <functional>
#include <random>

/**
* �Q�[���G���W���N���X.
*/
class GameEngine
{
public:
  typedef std::function<void(double)> UpdateFuncType; ///< �Q�[����Ԃ��X�V����֐��̌^.
                                                      /// �J�����f�[�^.
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

  //<--- �����Ƀ����o�֐���ǉ����� --->

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

  //<--- �����Ƀ����o�ϐ���ǉ����� --->
};
