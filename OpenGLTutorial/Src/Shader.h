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
typedef std::shared_ptr<Program> ProgramPtr; ///< �v���O�����I�u�W�F�N�g�|�C���^�^.

/**
* �V�F�[�_�[�v���O�����N���X.
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
  GLuint program = 0; ///< �v���O�����I�u�W�F�N�g.
  GLint samplerLocation = -1; ///< �T���v���[�̈ʒu.
  int samplerCount = 0; ///< �T���v���[�̐�.
  std::string name; ///< �v���O������.
};

GLuint CreateProgramFromFile(const char* vsFilename, const char* fsFilename);

} // namespace Shader

#endif