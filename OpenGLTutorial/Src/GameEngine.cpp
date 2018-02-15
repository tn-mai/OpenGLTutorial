/**
* @file GameEngine.cpp
*/
#include "GameEngine.h"
#include "GLFWEW.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

/**
* �Q�[���G���W���̃C���X�^���X���擾����.
*
* @return �Q�[���G���W���̃C���X�^���X.
*/
GameEngine& GameEngine::Instance()
{
  static GameEngine instance;
  return instance;
}

/**
* �Q�[���G���W��������������.
*
* @param w     �E�B���h�E�̕`��͈͂̕�(�s�N�Z��).
* @param h     �E�B���h�E�̕`��͈͂̍���(�s�N�Z��).
* @param title �E�B���h�E�^�C�g��(UTF-8��0�I�[������).
*
* @retval true  ����������.
* @retval false ���������s.
*
* Run�֐����Ăяo���O�ɁA��x�����Ăяo���Ă����K�v������.
* ��x�������ɐ�������ƁA�Ȍ�̌Ăяo���ł͂Ȃɂ�������true��Ԃ�.
*/
bool GameEngine::Init(int w, int h, const char* title)
{
  if (isInitialized) {
    return true;
  }
  if (!GLFWEW::Window::Instance().Init(w, h, title)) {
    return false;
  }

  //<--- �����ɏ�����������ǉ����� --->

  isInitialized = true;
  return true;
}

/**
* �Q�[�������s����.
*/
void GameEngine::Run()
{
  const double delta = 1.0 / 60.0;
  GLFWEW::Window& window = GLFWEW::Window::Instance();
  while (!window.ShouldClose()) {
    Update(delta);
    Render();
    window.SwapBuffers();
  }
}

/**
* ��ԍX�V�֐���ݒ肷��.
*
* @param func �ݒ肷��X�V�֐�.
*/
void GameEngine::UpdateFunc(const UpdateFuncType& func)
{
  updateFunc = func;
}

/**
* ��ԍX�V�֐����擾����.
*
* @return �ݒ肳��Ă���X�V�֐�.
*/
const GameEngine::UpdateFuncType& GameEngine::UpdateFunc() const
{
  return updateFunc;
}

/**
* �f�X�g���N�^.
*/
GameEngine::~GameEngine()
{
  //<--- �����ɏI��������ǉ����� --->
}

/**
* �Q�[���̏�Ԃ��X�V����.
*
* @param delta �O��̍X�V����̌o�ߎ���(�b).
*/
void GameEngine::Update(double delta)
{
  if (updateFunc) {
    updateFunc(delta);
  }
  //<--- �����ɍX�V������ǉ����� --->
}

/**
* �Q�[���̏�Ԃ�`�悷��.
*/
void GameEngine::Render() const
{
  //<--- �����ɕ`�揈����ǉ����܂� --->
}
