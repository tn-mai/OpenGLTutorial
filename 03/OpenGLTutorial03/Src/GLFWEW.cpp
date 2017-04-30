/**
* @file GLFWE.cpp
*/
#include "GLFWEW.h"
#include <iostream>
#include <iomanip>

/**
* GLFWとGLEWをラップするための名前空間.
*/
namespace GLFWEW {

namespace /* unnamed */ {

/**
* GLFWからのエラー報告を処理する.
*
* @param error エラー番号.
* @param desc  エラーの内容.
*/
void ErrorCallback(int error, const char* desc)
{
  std::cerr << "ERROR(0x" << std::hex << std::setfill('0') << std::setw(8) << error << "): " << desc << std::endl;
}

} // unnamed namespace

/**
* シングルトンインスタンスを取得する.
*
* @return Windowのシングルトンインスタンス.
*/
Window& Window::Instance()
{
  static Window instance;
  return instance;
}

/**
* コンストラクタ.
*/
Window::Window() : isGLFWInitialized(false), isInitialized(false), window(nullptr)
{
}

/**
* デストラクタ.
*/
Window::~Window()
{
  if (isGLFWInitialized) {
    glfwTerminate();
  }
}

/**
* GLFW/GLEWの初期化.
*
* @param w ウィンドウの描画範囲の幅(ピクセル).
* @param h ウィンドウの描画範囲の高さ(ピクセル).
* @param title ウィンドウタイトル(UTF-8の0終端文字列).
*
* @retval true 初期化成功.
* @retval false 初期化失敗.
*/
bool Window::Init(int w, int h, const char* title)
{
  if (isInitialized) {
    std::cerr << "ERROR: GLFWEWは既に初期化されています." << std::endl;
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
    std::cerr << "ERROR: GLEWの初期化に失敗しました." << std::endl;
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
* ウィンドウを閉じるべきか調べる.
*
* @retval true 閉じる.
* @retval false 閉じない. 
*/
bool Window::ShouldClose() const
{
  return glfwWindowShouldClose(window);
}

/**
* フロントバッファとバックバッファを切り替える.
*/
void Window::SwapBuffers() const
{
  glfwPollEvents();
  glfwSwapBuffers(window);
}

} // namespace GLFWEW
