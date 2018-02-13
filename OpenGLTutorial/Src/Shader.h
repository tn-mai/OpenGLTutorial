/**
* @file Shader.h
*/
#ifndef SHADER_H_INCLUDED
#define SHADER_H_INCLUDED
#include <GL/glew.h>
#include <string>
#include <memory>

namespace Shader {

class Program;
typedef std::shared_ptr<Program> ProgramPtr; ///< プログラムオブジェクトポインタ型.

/**
* シェーダープログラムクラス.
*/
class Program
{
public:
  static ProgramPtr Create(const char* vsFilename, const char* fsFilename);

  bool UniformBlockBinding(const char* blockName, GLuint bindingPoint);
  void UseProgram();
  void BindTexture(GLenum unit, GLenum type, GLuint texture);

private:
  Program() = default;
  ~Program();
  Program(const Program&) = delete;
  Program& operator=(const Program&) = delete;

private:
  GLuint program = 0; ///< プログラムオブジェクト.
  GLint samplerLocation = -1; ///< サンプラーの位置.
  int samplerCount = 0; ///< サンプラーの数.
  std::string name; ///< プログラム名.
};

GLuint CreateProgramFromFile(const char* vsFilename, const char* fsFilename);

} // namespace Shader

#endif