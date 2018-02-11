/**
* @file Main.cpp
*/
#include "GLFWEW.h"
#include <iostream>

/// エントリーポイント.
int main()
{
  GLFWEW::Window& window = GLFWEW::Window::Instance();
  if (!window.Init(800, 600, "OpenGL Tutorial")) {
    return 1;
  }

  // メインループ.
  while (!window.ShouldClose()) {
    glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    window.SwapBuffers();
  }

  return 0;
}