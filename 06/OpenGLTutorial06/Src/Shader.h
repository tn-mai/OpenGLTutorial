/**
* @file Shader.h
*/
#ifndef SHADER_H_INCLUDED
#define SHADER_H_INCLUDED
#include <GL/glew.h>

namespace Shader {

GLuint CreateProgram(const GLchar* vsCode, const GLchar* fsCode);
GLuint CreateProgramFromFile(const char* vsFilename, const char* fsFilename);

} // namespace Shader

#endif // SHADER_H_INCLUDED