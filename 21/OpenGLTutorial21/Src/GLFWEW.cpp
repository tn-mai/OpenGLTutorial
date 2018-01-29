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

/**
*
*/
void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
  static const char* const sourceNameList[] = { "API", "SYSTEM", "SHADER", "3RD", "APP", "OTHER" };
  const char* sourceName = sourceNameList[GL_DEBUG_SOURCE_OTHER - GL_DEBUG_SOURCE_API];
  if (source >= GL_DEBUG_SOURCE_API && source <= GL_DEBUG_SOURCE_OTHER) {
    sourceName = sourceNameList[source - GL_DEBUG_SOURCE_API];
  }

  static const char* const typeNameList[] = { "ERROR", "DEPRECATED", "UNDEFINED", "PORTABILITY", "PERFORMANCE", "OTHER" };
  const char* typeName = typeNameList[GL_DEBUG_TYPE_OTHER - GL_DEBUG_TYPE_ERROR];
  if (type >= GL_DEBUG_TYPE_ERROR && type <= GL_DEBUG_TYPE_OTHER) {
    typeName = typeNameList[type - GL_DEBUG_TYPE_ERROR];
  }

  static const char* const severityNameList[] = { "HIGH", "MEDIUM", "LOW" };
  const char* severityName = severityNameList[GL_DEBUG_SEVERITY_LOW - GL_DEBUG_SEVERITY_HIGH];
  if (severity >= GL_DEBUG_SEVERITY_HIGH && severity <= GL_DEBUG_SEVERITY_LOW) {
    severityName = severityNameList[severity - GL_DEBUG_SEVERITY_HIGH];
  }

  std::cerr << typeName << "(" << sourceName << ", " << severityName << "): " << message << std::endl;
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
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
    //window = glfwCreateWindow(w, h, title, glfwGetPrimaryMonitor(), nullptr);
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

  glDebugMessageCallback(DebugCallback, nullptr);

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

/**
* �Q�[���p�b�h�̏�Ԃ��擾����.
*
* @param id �擾����Q�[���p�b�h��ID.
*
* @return id�ɑΉ�����Q�[���p�b�h.
*/
const GamePad& Window::GetGamePad(int id) const
{
  return gamepad[id];
}

/**
* �W���C�X�e�B�b�N�̃A�i���O���͑��uID.
+
* @note XBOX360�R���g���[���[�.
*/
enum GLFWAXESID {
  GLFWAXESID_LeftX, ///< ���X�e�B�b�N��X��.
  GLFWAXESID_LeftY, ///< ���X�e�B�b�N��Y��.
  GLFWAXESID_BackX, ///< �A�i���O�g���K�[.
  GLFWAXESID_RightY, ///< �E�X�e�B�b�N��Y��.
  GLFWAXESID_RightX, ///< �E�X�e�B�b�N��X��.
};

/**
* �W���C�X�e�B�b�N�̃f�W�^�����͑��uID.
+
* @note XBOX360�R���g���[���[�.
*/
enum GLFWBUTTONID {
  GLFWBUTTONID_A, ///< A�{�^��.
  GLFWBUTTONID_B, ///< B�{�^��.
  GLFWBUTTONID_X, ///< X�{�^��.
  GLFWBUTTONID_Y, ///< Y�{�^��.
  GLFWBUTTONID_L, ///< L�{�^��.
  GLFWBUTTONID_R, ///< R�{�^��.
  GLFWBUTTONID_Back, ///< Back�{�^��.
  GLFWBUTTONID_Start, ///< Start�{�^��.
  GLFWBUTTONID_LThumb, ///< ���X�e�B�b�N�{�^��.
  GLFWBUTTONID_RThumb, ///< �E�X�e�B�b�N�{�^��.
  GLFWBUTTONID_Up, ///< ��L�[.
  GLFWBUTTONID_Right, ///< �E�L�[.
  GLFWBUTTONID_Down, ///< ���L�[.
  GLFWBUTTONID_Left, ///< ���L�[.
};

/**
* �Q�[���p�b�h�̏�Ԃ��X�V����.
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
  const int key = glfwGetKey(window, GLFW_KEY_ESCAPE);
  if (key == GLFW_PRESS) {
    gamepad[0].buttons |= GamePad::ESC;
  } else if (key == GLFW_RELEASE) {
    gamepad[0].buttons &= ~GamePad::ESC;
  }

  gamepad[0].buttonDown = gamepad[0].buttons & ~prevButtons;
}

void Window::Close()
{
  glfwSetWindowShouldClose(window, 1);
}

Window::KeyState Window::GetKey(int code) const
{
  return glfwGetKey(window, code) == GLFW_PRESS ? KeyState::Press : KeyState::Release;
}

int Window::Width() const
{
  int w, h;
  glfwGetWindowSize(window, &w, &h);
  return w;
}

int Window::Height() const
{
  int w, h;
  glfwGetWindowSize(window, &w, &h);
  return h;
}

} // namespace GLFWEW
