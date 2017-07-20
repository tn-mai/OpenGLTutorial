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
  const GLubyte* extensions = glGetString(GL_EXTENSIONS);
  std::cout << "Extensions: " << extensions << std::endl;

  GLint param;
  glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK_LEFT, GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING, &param);
  std::cout << "Color Encoding: " << (param == GL_SRGB ? "GL_SRGB" : "GL_LINEAR") << std::endl;

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

/**
* ゲームパッドの状態を取得する.
*
* @param id 取得するゲームパッドのID.
*
* @return idに対応するゲームパッド.
*/
const GamePad& Window::GetGamePad(int id) const
{
  return gamepad[id];
}

/**
* ジョイスティックのアナログ入力装置ID.
+
* @note XBOX360コントローラー基準.
*/
enum GLFWAXESID {
  GLFWAXESID_LeftX, ///< 左スティックのX軸.
  GLFWAXESID_LeftY, ///< 左スティックのY軸.
  GLFWAXESID_BackX, ///< アナログトリガー.
  GLFWAXESID_RightY, ///< 右スティックのY軸.
  GLFWAXESID_RightX, ///< 右スティックのX軸.
};

/**
* ジョイスティックのデジタル入力装置ID.
+
* @note XBOX360コントローラー基準.
*/
enum GLFWBUTTONID {
  GLFWBUTTONID_A, ///< Aボタン.
  GLFWBUTTONID_B, ///< Bボタン.
  GLFWBUTTONID_X, ///< Xボタン.
  GLFWBUTTONID_Y, ///< Yボタン.
  GLFWBUTTONID_L, ///< Lボタン.
  GLFWBUTTONID_R, ///< Rボタン.
  GLFWBUTTONID_Back, ///< Backボタン.
  GLFWBUTTONID_Start, ///< Startボタン.
  GLFWBUTTONID_LThumb, ///< 左スティックボタン.
  GLFWBUTTONID_RThumb, ///< 右スティックボタン.
  GLFWBUTTONID_Up, ///< 上キー.
  GLFWBUTTONID_Right, ///< 右キー.
  GLFWBUTTONID_Down, ///< 下キー.
  GLFWBUTTONID_Left, ///< 左キー.
};

/**
* ゲームパッドの状態を更新する.
*/
void Window::UpdateGamePad()
{
  const uint32_t prevButtons = gamepad[0].buttons;
  int axesCount, buttonCount;
  const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
  const uint8_t* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);
  if (axes && buttons && axesCount >= 2 && buttonCount >= 8) {
    gamepad[0].buttons &= ~(GamePad::DPAD_UP | GamePad::DPAD_DOWN | GamePad::DPAD_LEFT | GamePad::DPAD_RIGHT);
    static const float threshould = 0.3f;
    if (axes[GLFWAXESID_LeftY] >= threshould) {
      gamepad[0].buttons |= GamePad::DPAD_UP;
    } else if (axes[GLFWAXESID_LeftY] <= -threshould) {
      gamepad[0].buttons |= GamePad::DPAD_DOWN;
    }
    if (axes[GLFWAXESID_LeftX] >= threshould) {
      gamepad[0].buttons |= GamePad::DPAD_LEFT;
    } else if (axes[GLFWAXESID_LeftX] <= -threshould) {
      gamepad[0].buttons |= GamePad::DPAD_RIGHT;
    }
    static const struct {
      int glfwCode;
      int gamepadCode;
    } keyMap[] = {
      { GLFWBUTTONID_A, GamePad::A },
      { GLFWBUTTONID_B, GamePad::B },
      { GLFWBUTTONID_X, GamePad::X },
      { GLFWBUTTONID_Y, GamePad::Y },
      { GLFWBUTTONID_Start, GamePad::START },
    };
    for (const auto& e : keyMap) {
      if (buttons[e.glfwCode] == GLFW_PRESS) {
        gamepad[0].buttons |= e.gamepadCode;
      } else if (buttons[e.glfwCode] == GLFW_RELEASE) {
        gamepad[0].buttons &= ~e.gamepadCode;
      }
    }
  } else {
    static const struct {
      int glfwCode;
      uint32_t gamepadCode;
    } keyMap[] = {
      { GLFW_KEY_UP, GamePad::DPAD_UP },
      { GLFW_KEY_DOWN, GamePad::DPAD_DOWN },
      { GLFW_KEY_LEFT, GamePad::DPAD_LEFT },
      { GLFW_KEY_RIGHT, GamePad::DPAD_RIGHT },
      { GLFW_KEY_ENTER, GamePad::START },
      { GLFW_KEY_A, GamePad::A },
      { GLFW_KEY_S, GamePad::B },
      { GLFW_KEY_Z, GamePad::X },
      { GLFW_KEY_X, GamePad::Y },
    };
    for (const auto& e : keyMap) {
      const int key = glfwGetKey(window, e.glfwCode);
      if (key == GLFW_PRESS) {
        gamepad[0].buttons |= e.gamepadCode;
      } else if (key == GLFW_RELEASE) {
        gamepad[0].buttons &= ~e.gamepadCode;
      }
    }
  }
  gamepad[0].buttonDown = gamepad[0].buttons & ~prevButtons;
}

} // namespace GLFWEW
