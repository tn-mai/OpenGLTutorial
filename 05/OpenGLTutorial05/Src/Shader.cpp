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
* �V�F�[�_�R�[�h���R���p�C������.
*
* @param type �V�F�[�_�̎��.
* @param string �V�F�[�_�R�[�h�ւ̃|�C���^.
*
* @return �쐬�����V�F�[�_�I�u�W�F�N�g.
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
				std::cerr << "ERROR: �V�F�[�_�̃R���p�C���Ɏ��s" << buf.data() << std::endl;
			}
			glDeleteShader(shader);
		}
		return 0;
	}
	return shader;
}

/**
* �V�F�[�_�v���O�������쐬����.
*
* @param vsCode ���_�V�F�[�_�R�[�h�ւ̃|�C���^.
* @param fsCode �t���O�����g�V�F�[�_�R�[�h�ւ̃|�C���^.
*
* @return �쐬�����v���O�����I�u�W�F�N�g.
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
				std::cerr << "ERROR: �V�F�[�_�̃����N�Ɏ��s" << buf.data() << std::endl;
			}
		}
		glDeleteProgram(program);
		return 0;
	}
	return program;
}

/**
* �t�@�C����ǂݍ���.
*
* @param filename �ǂݍ��ރt�@�C����.
* @param buf      �ǂݍ��ݐ�o�b�t�@.
*
* @retval true �ǂݍ��ݐ���.
* @retval false �ǂݍ��ݎ��s.
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
* �t�@�C������V�F�[�_�v���O�������쐬����.
*
* @param vsCode ���_�V�F�[�_�t�@�C����.
* @param fsCode �t���O�����g�V�F�[�_�t�@�C����.
*
* @return �쐬�����v���O�����I�u�W�F�N�g.
*/
GLuint CreateProgramFromFile(const char* vsFilename, const char* fsFilename)
{
	std::vector<char> vsBuf;
	if (!ReadFile(vsFilename, vsBuf)) {
		std::cerr << "ERROR in Shader::CreateProgramFromFile:\n" << vsFilename << "��ǂݍ��߂܂���." << std::endl;
		return 0;
	}
	std::vector<char> fsBuf;
	if (!ReadFile(fsFilename, fsBuf)) {
		std::cerr << "ERROR in Shader::CreateProgramFromFile:\n" << fsFilename << "��ǂݍ��߂܂���." << std::endl;
		return 0;
	}
	return CreateProgram(vsBuf.data(), fsBuf.data());
}

} // namespace Shader