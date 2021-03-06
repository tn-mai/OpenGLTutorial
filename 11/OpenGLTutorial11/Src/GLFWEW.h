/**
* @file GLFWEW.h
*/
#ifndef GLFWEW_INCLUDED
#define GLFWEW_INCLUDED
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "GamePad.h"

namespace GLFWEW {

/**
* GLFWとGLEWのラッパークラス.
*/
class Window {
public:
  static Window& Instance();
  bool Init(int w, int h, const char* title);
  bool ShouldClose() const;
  void SwapBuffers() const;
  void UpdateGamePad();
  const GamePad& GetGamePad(int id) const;

private:
  Window();
  ~Window();
  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;

  bool isGLFWInitialized;
  bool isInitialized;
  GLFWwindow* window;
  GamePad gamepad[4];
};

} // namespace GLFWEW

#endif // GLFWEW_INCLUDED