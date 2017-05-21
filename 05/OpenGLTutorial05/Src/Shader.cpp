/**
* @file Shader.cpp
*/
#include "Shader.h"
#include <vector>
#include <iostream>
#include <cstdint>
#include <stdio.h>
#include <sys/stat.h>

namespace Shader {

/**
* シェーダコードをコンパイルする.
*
* @param type シェーダの種類.
* @param string シェーダコードへのポインタ.
*
* @return 作成したシェーダオブジェクト.
*/
GLuint CompileShader(GLenum type, const GLchar* string)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &string, nullptr);
	glCompileShader(shader);
	GLint compiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		GLint infoLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen) {
			std::vector<char> buf;
			buf.resize(infoLen);
			if (static_cast<int>(buf.size()) >= infoLen) {
				glGetShaderInfoLog(shader, infoLen, NULL, buf.data());
				std::cerr << "ERROR: シェーダのコンパイルに失敗" << buf.data() << std::endl;
			}
			glDeleteShader(shader);
		}
		return 0;
	}
	return shader;
}

/**
* シェーダプログラムを作成する.
*
* @param vsCode 頂点シェーダコードへのポインタ.
* @param fsCode フラグメントシェーダコードへのポインタ.
*
* @return 作成したプログラムオブジェクト.
*/
GLuint CreateProgram(const GLchar* vsCode, const GLchar* fsCode)
{
	GLuint vs = CompileShader(GL_VERTEX_SHADER, vsCode);
	GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsCode);
	if (!vs || !fs) {
		return 0;
	}
	GLuint program = glCreateProgram();
	glAttachShader(program, fs);
	glDeleteShader(fs);
	glAttachShader(program, vs);
	glDeleteShader(vs);
	glLinkProgram(program);
	GLint linkStatus = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	if (linkStatus != GL_TRUE) {
		GLint infoLen = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen) {
			std::vector<char> buf;
			buf.resize(infoLen);
			if (static_cast<int>(buf.size()) >= infoLen) {
				glGetProgramInfoLog(program, infoLen, NULL, buf.data());
				std::cerr << "ERROR: シェーダのリンクに失敗" << buf.data() << std::endl;
			}
		}
		glDeleteProgram(program);
		return 0;
	}
	return program;
}

/**
* ファイルを読み込む.
*
* @param filename 読み込むファイル名.
* @param buf      読み込み先バッファ.
*
* @retval true 読み込み成功.
* @retval false 読み込み失敗.
*/
bool ReadFile(const char* filename, std::vector<char>& buf)
{
	struct stat st;
	if (stat(filename, &st)) {
		return false;
	}
	FILE* fp = fopen(filename, "rb");
	if (!fp) {
		return false;
	}
	buf.resize(st.st_size);
	const size_t readSize = fread(buf.data(), 1, st.st_size, fp);
	fclose(fp);
	if (readSize != st.st_size) {
		return false;
	}
	return true;
}

/**
* ファイルからシェーダプログラムを作成する.
*
* @param vsCode 頂点シェーダファイル名.
* @param fsCode フラグメントシェーダファイル名.
*
* @return 作成したプログラムオブジェクト.
*/
GLuint CreateProgramFromFile(const char* vsFilename, const char* fsFilename)
{
	std::vector<char> vsBuf;
	if (!ReadFile(vsFilename, vsBuf)) {
		std::cerr << "ERROR in Shader::CreateProgramFromFile:\n" << vsFilename << "を読み込めません." << std::endl;
		return 0;
	}
	std::vector<char> fsBuf;
	if (!ReadFile(fsFilename, fsBuf)) {
		std::cerr << "ERROR in Shader::CreateProgramFromFile:\n" << fsFilename << "を読み込めません." << std::endl;
		return 0;
	}
	return CreateProgram(vsBuf.data(), fsBuf.data());
}

} // namespace Shader