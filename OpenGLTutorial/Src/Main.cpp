/**
* @file Main.cpp
*/
#include <GL/glew.h>
#include <GLFW\glfw3.h>
#include <iostream>

/**
* GLFWからのエラー報告を処理する.
*
* @param error エラー番号.
* @param desc  エラーの内容.
*/
void ErrorCallback(int error, const char* desc)
{
  std::cerr << "ERROR: " << desc << std::endl;
}

/// エントリーポイント.
int main()
{
  // GLFWの初期化.
  glfwSetErrorCallback(ErrorCallback);
  if (glfwInit() != GL_TRUE) {
    return 1;
  }
  GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Tutorial", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);

  // GLEWの初期化.
  if (glewInit() != GLEW_OK) {
    std::cerr << "ERROR: GLEWの初期化に失敗しました." << std::endl;
    glfwTerminate();
    return 1;
  }

  // メインループ.
  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  // GLFWの終了.
  glfwTerminate();

  return 0;
}