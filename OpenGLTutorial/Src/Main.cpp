/**
* @file Main.cpp
*/
#include <GL/glew.h>
#include <GLFW\glfw3.h>
#include <iostream>

/**
* GLFW����̃G���[�񍐂���������.
*
* @param error �G���[�ԍ�.
* @param desc  �G���[�̓��e.
*/
void ErrorCallback(int error, const char* desc)
{
  std::cerr << "ERROR: " << desc << std::endl;
}

/// �G���g���[�|�C���g.
int main()
{
  // GLFW�̏�����.
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

  // GLEW�̏�����.
  if (glewInit() != GLEW_OK) {
    std::cerr << "ERROR: GLEW�̏������Ɏ��s���܂���." << std::endl;
    glfwTerminate();
    return 1;
  }

  // ���C�����[�v.
  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  // GLFW�̏I��.
  glfwTerminate();

  return 0;
}