/**
* @file GLFWE.cpp
*/
#include "GLFWEW.h"
#include <iostream>
#include <iomanip>

/**
* GLFW��GLEW�����b�v���邽�߂̖��O���.
*/
namespace GLFWEW {

namespace /* unnamed */ {

/**
* GLFW����̃G���[�񍐂���������.
*
* @param error �G���[�ԍ�.
* @param desc  �G���[�̓��e.
*/
void ErrorCallback(int error, const char* desc)
{
  std::cerr << "ERROR(0x" << std::hex << std::setfill('0') << std::setw(8) << error << "): " << desc << std::endl;
}

} // unnamed namespace

/**
* �V���O���g���C���X�^���X���擾����.
*
* @return Window�̃V���O���g���C���X�^���X.
*/
Window& Window::Instance()
{
  static Window instance;
  return instance;
}

/**
* �R���X�g���N�^.
*/
Window::Window() : isGLFWInitialized(false), isInitialized(false), window(nullptr)
{
}

/**
* �f�X�g���N�^.
*/
Window::~Window()
{
  if (isGLFWInitialized) {
    glfwTerminate();
  }
}

/**
* GLFW/GLEW�̏�����.
*
* @param w �E�B���h�E�̕`��͈͂̕�(�s�N�Z��).
* @param h �E�B���h�E�̕`��͈͂̍���(�s�N�Z��).
* @param title �E�B���h�E�^�C�g��(UTF-8��0�I�[������).
*
* @retval true ����������.
* @retval false ���������s.
*/
bool Window::Init(int w, int h, const char* title)
{
  if (isInitialized) {
    std::cerr << "ERROR: GLFWEW�͊��ɏ���������Ă��܂�." << std::endl;
    return false;
  }
  if (!isGLFWInitialized) {
    glfwSetErrorCallback(ErrorCallback);
    if (glfwInit() != GL_TRUE) {
      return false;
    }
    isGLFWInitialized = true;
  }

  if (!window) {
    window = glfwCreateWindow(w, h, title, nullptr, nullptr);
    if (!window) {
      return false;
    }
    glfwMakeContextCurrent(window);
  }

  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    std::cerr << "ERROR: GLEW�̏������Ɏ��s���܂���." << std::endl;
    return false;
  }
  const GLubyte* renderer = glGetString(GL_RENDERER);
  std::cout << "Renderer: " << renderer << std::endl;
  const GLubyte* version = glGetString(GL_VERSION);
  std::cout << "Version: " << version << std::endl;
  isInitialized = true;
  return true;
}

/**
* �E�B���h�E�����ׂ������ׂ�.
*
* @retval true ����.
* @retval false ���Ȃ�. 
*/
bool Window::ShouldClose() const
{
  return glfwWindowShouldClose(window);
}

/**
* �t�����g�o�b�t�@�ƃo�b�N�o�b�t�@��؂�ւ���.
*/
void Window::SwapBuffers() const
{
  glfwPollEvents();
  glfwSwapBuffers(window);
}

} // namespace GLFWEW
