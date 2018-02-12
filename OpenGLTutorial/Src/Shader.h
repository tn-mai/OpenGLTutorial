/**
* @file Shader.h
*/
#ifndef SHADER_H_INCLUDED
#define SHADER_H_INCLUDED
#include <GL/glew.h>

namespace Shader {

GLuint CreateProgramFromFile(const char* vsFilename, const char* fsFilename);

} // namespace Shader

#endif