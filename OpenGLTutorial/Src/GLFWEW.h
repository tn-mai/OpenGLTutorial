/**
* @file GLFWEW.h
*/
#ifndef GLFWEW_H_INCLUDED
#define GLFWEW_H_INCLUDED
#include <GL/glew.h>
#include "GamePad.h"
#include <GLFW/glfw3.h>

namespace GLFWEW {

/**
* GLFWとGLEWのラッパークラス.
*/
class Window
{
public:
  static Window& Instance();
  bool Init(int w, int h, const char* title);
  bool ShouldClose() const;
  void SwapBuffers() const;
  const GamePad& GetGamePad() const;
  void UpdateGamePad();

private:
  Window()= default;
  ~Window();
  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  bool isGLFWInitialized = false;
  bool isInitialized = false;
  GLFWwindow* window = nullptr;
  GamePad gamepad;
};

} // namespace GLFWEW

#endif // GLFWEW_H_INCLUDED