/**
* @file Main.cpp
*/
#include "GLFWEW.h"
#include "Texture.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

/// ���_�f�[�^�^.
struct Vertex
{
  glm::vec3 position; ///< ���W.
  glm::vec4 color; ///< �F.
  glm::vec2 texCoord; ///< �e�N�X�`�����W.
};

/// ���_�f�[�^.
const Vertex vertices[] = {
  { {-0.5f, -0.3f, 0.5f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f} },
  { { 0.3f, -0.3f, 0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f} },
  { { 0.3f,  0.5f, 0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f} },
  { {-0.5f,  0.5f, 0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f} },

  { {-0.3f,  0.3f, 0.1f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f} },
  { {-0.3f, -0.5f, 0.1f}, {0.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} },
  { { 0.5f, -0.5f, 0.1f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f} },
  { { 0.5f, -0.5f, 0.1f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f} },
  { { 0.5f,  0.3f, 0.1f}, {1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f} },
  { {-0.3f,  0.3f, 0.1f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f} },
};

/// �C���f�b�N�X�f�[�^.
const GLuint indices[] = {
  0, 1, 2, 2, 3, 0,
  4, 5, 6, 7, 8, 9,
};

/// ���_�V�F�[�_.
static const char* vsCode =
"#version 410 \n"
"layout(location=0) in vec3 vPosition;"
"layout(location=1) in vec4 vColor;"
"layout(location=2) in vec2 vTexCoord;"
"layout(location=0) out vec4 outColor;"
"layout(location=1) out vec2 outTexCoord;"
"uniform mat4x4 matMVP;"
"void main() {"
"  outColor = vColor;"
"  outTexCoord = vTexCoord;"
"  gl_Position = matMVP * vec4(vPosition, 1.0);"
"}";

/// �t���O�����g�V�F�[�_.
static const char* fsCode =
"#version 410 \n"
"layout(location=0) in vec4 inColor;"
"layout(location=1) in vec2 inTexCoord;"
"uniform sampler2D colorSampler;"
"out vec4 fragColor;"
"void main() {"
"  fragColor = inColor * texture(colorSampler, inTexCoord);"
"}";

/**
* Vertex Buffer Object���쐬����.
*
* @param size ���_�f�[�^�̃T�C�Y.
* @param data ���_�f�[�^�ւ̃|�C���^.
*
* @return �쐬����VBO.
*/
GLuint CreateVBO(GLsizeiptr size, const GLvoid* data)
{
  GLuint vbo = 0;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  return vbo;
}

/**
* Index Buffer Object���쐬����.
*
* @param size �C���f�b�N�X�f�[�^�̃T�C�Y.
* @param data �C���f�b�N�X�f�[�^�ւ̃|�C���^.
*
* @return �쐬����IBO.
*/
GLuint CreateIBO(GLsizeiptr size, const GLvoid* data)
{
  GLuint ibo = 0;
  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  return ibo;
}

/**
* ���_�A�g���r���[�g��ݒ肷��.
*
* @param index ���_�A�g���r���[�g�̃C���f�b�N�X.
* @param cls   ���_�f�[�^�^��.
* @param mbr   ���_�A�g���r���[�g�ɐݒ肷��cls�̃����o�ϐ���.
*/
#define SetVertexAttribPointer(index, cls, mbr) SetVertexAttribPointerI( \
  index, \
  sizeof(cls::mbr) / sizeof(float), \
  sizeof(cls), \
  reinterpret_cast<GLvoid*>(offsetof(cls, mbr)))

void SetVertexAttribPointerI(GLuint index, GLint size, GLsizei stride, const GLvoid* pointer)
{
  glEnableVertexAttribArray(index);
  glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride, pointer);
}

/**
* Vertex Array Object���쐬����.
*
* @param vbo VAO�Ɋ֘A�t������VBO.
* @param ibo VAO�Ɋ֘A�t������IBO.
*
* @return �쐬����VAO.
*/
GLuint CreateVAO(GLuint vbo, GLuint ibo)
{
  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  SetVertexAttribPointer(0, Vertex, position);
  SetVertexAttribPointer(1, Vertex, color);
  SetVertexAttribPointer(2, Vertex, texCoord);
  glBindVertexArray(0);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ibo);
  return vao;
}

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
        std::cerr << "ERROR: �V�F�[�_�̃R���p�C���Ɏ��s��n" << buf.data() << std::endl;
      }
    }
    glDeleteShader(shader);
    return 0;
  }
  return shader;
}

/**
* �v���O�����I�u�W�F�N�g���쐬����.
*
* @param vsCode ���_�V�F�[�_�R�[�h�ւ̃|�C���^.
* @param fsCode �t���O�����g�V�F�[�_�R�[�h�ւ̃|�C���^.
*
* @return �쐬�����v���O�����I�u�W�F�N�g.
*/
GLuint CreateShaderProgram(const GLchar* vsCode, const GLchar* fsCode)
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
        std::cerr << "ERROR: �V�F�[�_�̃����N�Ɏ��s��n"<< buf.data() << std::endl;
      }
    }
    glDeleteProgram(program);
    return 0;
  }
  return program;
}

/// �G���g���[�|�C���g.
int main()
{
  GLFWEW::Window& window = GLFWEW::Window::Instance();
  if (!window.Init(800, 600, "OpenGL Tutorial")) {
    return 1;
  }

  const GLuint vbo = CreateVBO(sizeof(vertices), vertices);
  const GLuint ibo = CreateIBO(sizeof(indices), indices);
  const GLuint vao = CreateVAO(vbo, ibo);
  const GLuint shaderProgram = CreateShaderProgram(vsCode, fsCode);
  if (!vbo || !ibo || !vao || !shaderProgram) {
    return 1;
  }

  // �e�N�X�`���f�[�^.
  TexturePtr tex = Texture::LoadFromFile("Res/sample.bmp");
  if (!tex) {
    return 1;
  }

  glEnable(GL_DEPTH_TEST);

  // ���C�����[�v.
  while (!window.ShouldClose()) {
    glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ���_����]�ړ�������.
    static float degree = 0.0f;
    degree += 0.1f;
    if (degree >= 360.0f) { degree -= 360.0f; }
    const glm::vec3 viewPos = glm::rotate(glm::mat4(), glm::radians(degree), glm::vec3(0, 1, 0)) * glm::vec4(2, 3, 3, 1);

    glUseProgram(shaderProgram);
    const GLint matMVPLoc = glGetUniformLocation(shaderProgram, "matMVP");
    if (matMVPLoc >= 0) {
      const glm::mat4x4 matProj =
        glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
      const glm::mat4x4 matView =
        glm::lookAt(viewPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
      const glm::mat4x4 matMVP = matProj * matView;
      glUniformMatrix4fv(matMVPLoc, 1, GL_FALSE, &matMVP[0][0]);
    }
    const GLint colorSamplerLoc = glGetUniformLocation(shaderProgram, "colorSampler");
    if (colorSamplerLoc >= 0) {
      glUniform1i(colorSamplerLoc, 0);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, tex->Id());
    }
    glBindVertexArray(vao);
    glDrawElements(
      GL_TRIANGLES, sizeof(indices)/sizeof(indices[0]),
      GL_UNSIGNED_INT, reinterpret_cast<const GLvoid*>(0));

    window.SwapBuffers();
  }

  glDeleteProgram(shaderProgram);
  glDeleteVertexArrays(1, &vao);

  return 0;
}