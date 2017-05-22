/**
* @file main.cpp
*/
#include "GLFWEW.h"
#include "Texture.h"
#include "Shader.h"
#include "OffscreenBuffer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

/// 頂点データ型.
struct Vertex
{
  glm::vec3 position; ///< 座標
  glm::vec4 color; ///< 色
  glm::vec2 texCoord; ///< テクスチャ座標.
};

/// 頂点データ.
const Vertex vertices[] = {
  { {-0.5f, -0.3f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, { 0.0f, 0.0f} },
  { { 0.3f, -0.3f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, { 1.0f, 0.0f} },
  { { 0.3f,  0.5f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, { 1.0f, 1.0f} },
  { {-0.5f,  0.5f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, { 0.0f, 1.0f} },

  { {-0.3f,  0.3f, 0.1f}, {0.0f, 0.0f, 1.0f, 1.0f}, { 0.0f, 1.0f} },
  { {-0.3f, -0.5f, 0.1f}, {0.0f, 1.0f, 1.0f, 1.0f}, { 0.0f, 0.0f} },
  { { 0.5f, -0.5f, 0.1f}, {0.0f, 0.0f, 1.0f, 1.0f}, { 1.0f, 0.0f} },
  { { 0.5f, -0.5f, 0.1f}, {1.0f, 0.0f, 0.0f, 1.0f}, { 1.0f, 0.0f} },
  { { 0.5f,  0.3f, 0.1f}, {1.0f, 1.0f, 0.0f, 1.0f}, { 1.0f, 1.0f} },
  { {-0.3f,  0.3f, 0.1f}, {1.0f, 0.0f, 0.0f, 1.0f}, { 0.0f, 1.0f} },
};

/// インデックスデータ.
const GLuint indices[] = {
  0, 1, 2, 2, 3, 0,
  4, 5, 6, 7, 8, 9,
};

/**
* Vertex Buffer Objectを作成する.
*
* @param size 頂点データのサイズ.
* @param data 頂点データへのポインタ.
*
* @return 作成したVBOのID.
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
* Index Buffer Objectを作成する.
*
* @param size インデックスデータのサイズ.
* @param data インデックスデータへのポインタ.
*
* @return 作成したIBO.
*/
GLuint CreateIBO(GLsizeiptr size, const GLvoid* data)
{
  GLuint ibo = 0;
  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STREAM_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  return ibo;
}

/**
* 頂点アトリビュートを設定する.
*
* @param index 頂点アトリビュートのインデックス.
* @param cls   頂点データ型名.
* @param mbr   頂点アトリビュートに設定するclsのメンバ変数名.
*/
#define SetVertexAttribPointer(index, cls, mbr) \
  SetVertexAttribPointerI(index, sizeof(cls::mbr) / sizeof(float), sizeof(cls), reinterpret_cast<GLvoid*>(offsetof(cls, mbr)))
void SetVertexAttribPointerI(GLuint index, GLint size, GLsizei stride, const GLvoid* pointer)
{
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride, pointer);
}

/**
* Vertex Array Objectを作成する.
*
* @param vbo VAOに関連付けられるVBO.
* @param ibo VAOに関連付けられるIBO.
*
* @return 作成したVAO.
*/
GLuint CreateVAO(GLuint vbo, GLuint ibo)
{
  GLuint vao = 0;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  SetVertexAttribPointer(0, Vertex, position);
  SetVertexAttribPointer(1, Vertex, color);
  SetVertexAttribPointer(2, Vertex, texCoord);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBindVertexArray(0);
  return vao;
}

struct PointLight
{
  glm::vec4 position;
  glm::vec4 color;
};

struct TransformationData
{
  glm::mat4 matMV;
  glm::mat4 matMVP;
  glm::mat4 matTex;
  PointLight light;
};

struct LightingData
{
  glm::vec4 ambientColor;
  PointLight light[4];
};

/**
* Uniform Block Objectを作成する.
*
* @param size Uniform Blockのサイズ.
* @param data Uniform Blockに転送するデータへのポインタ.
*
* @return 作成したUBO.
*/
GLuint CreateUBO(GLsizeiptr size, const GLvoid* data = nullptr)
{
  GLuint ubo;
  glGenBuffers(1, &ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, ubo);
  glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  return ubo;
}

/// エントリーポイント.
int main()
{
  GLFWEW::Window& window = GLFWEW::Window::Instance();
  if (!window.Init(800, 600, "OpenGL Tutorial")) {
    return 1;
  }

  const GLuint vbo = CreateVBO(sizeof(vertices), vertices);
  const GLuint ibo = CreateIBO(sizeof(indices), indices);
  const GLuint vao = CreateVAO(vbo, ibo);
  const GLuint uboTrans = CreateUBO(sizeof(TransformationData));
  const GLuint shaderProgram = Shader::CreateProgramFromFile("Res/Tutorial.vert", "Res/Tutorial.frag");
  if (!vbo || !ibo || !vao || !uboTrans || !shaderProgram) {
	  return 1;
  }

  static const uint32_t textureData[] = {
    0xffffffff, 0xffcccccc, 0xffffffff, 0xffcccccc, 0xffffffff,
    0xff888888, 0xffffffff, 0xff888888, 0xffffffff, 0xff888888,
    0xffffffff, 0xff444444, 0xffffffff, 0xff444444, 0xffffffff,
    0xff000000, 0xffffffff, 0xff000000, 0xffffffff, 0xff000000,
    0xffffffff, 0xff000000, 0xffffffff, 0xff000000, 0xffffffff,
  };
//  TexturePtr tex = Texture::Create(5, 5, GL_RGBA8, GL_RGBA, textureData);
  TexturePtr tex = Texture::LoadFromFile("Res/Sample.bmp");
  if (!tex) {
    return 1;
  }

  glEnable(GL_DEPTH_TEST);
//  glEnable(GL_CULL_FACE);

  const GLuint ubiTrans = glGetUniformBlockIndex(shaderProgram, "TransformationData");
  const GLuint ubiLight = glGetUniformBlockIndex(shaderProgram, "LightingData");
  if (ubiTrans == GL_INVALID_INDEX || ubiLight == GL_INVALID_INDEX) {
    std::cerr << "ERROR: Uniformブロックが見つかりません" << std::endl;
    return 1;
  }
  glUniformBlockBinding(shaderProgram, ubiTrans, 0);
  glUniformBlockBinding(shaderProgram, ubiLight, 1);

  const GLuint uboLight = CreateUBO(sizeof(LightingData));

  int bufferIndex = 0;
  LightingData lightData[2];

  const OffscreenBufferPtr offscreen = OffscreenBuffer::Create(800, 600);

  while (!window.ShouldClose()) {
    glBindFramebuffer(GL_FRAMEBUFFER, offscreen->GetFramebuffer());

    glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static float degree = 0.0f;
//    degree += 0.05f;
    if (degree >= 360.0f) { degree -= 360.0f; }
    const glm::vec3 viewPos = glm::rotate(glm::mat4(), glm::radians(degree), glm::vec3(0, 1, 0)) * glm::vec4(2, 3, 3, 1);

    glUseProgram(shaderProgram);

    {
      static float texRot = 0;
      //texRot += 0.05f;
      if (texRot >= 360) { texRot -= 360; }
      const glm::mat4x4 matProj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
      const glm::mat4x4 matView = glm::lookAt(viewPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
      const glm::mat4x4 matModel = glm::scale(glm::mat4(), glm::vec3(2, 2, 2));

      TransformationData transData;
      transData.matMV = matView * matModel;
      transData.matMVP = matProj * transData.matMV;
      glm::mat4 matTex = glm::translate(glm::mat4(), glm::vec3(0.5f, 0.5f, 0));
      matTex = glm::rotate(matTex, glm::radians(texRot), glm::vec3(0, 0, 1));
      matTex = glm::translate(matTex, glm::vec3(-0.5f, -0.5f, 0));
      transData.matTex = matTex;

      transData.light.color = glm::vec4(0.75f, 0.75f, 1, 1) * 5.0f;
      transData.light.position = glm::vec4(1, 1, 1, 1);

      lightData[bufferIndex].ambientColor = glm::vec4(0.05f, 0.15f, 0.25f, 1) * 0.5f;
      for (auto& e : lightData[bufferIndex].light) {
        e.color = glm::vec4(0, 0, 0, 0);
        e.position = glm::vec4(0, 0, 0, 0);
      }
      lightData[bufferIndex].light[0].color = glm::vec4(1, 1, 0.75f, 1) * 0.5f;
      lightData[bufferIndex].light[0].position = transData.matMV * glm::vec4(0, 0, 0, 1);
      lightData[bufferIndex].light[1].color = glm::vec4(0.75f, 0.75f, 1, 1) * 2.0f;
      lightData[bufferIndex].light[1].position = transData.matMV * glm::vec4(-0.5f, 1, 0, 1);
      glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboTrans);
      glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(TransformationData), &transData);
      glBindBufferBase(GL_UNIFORM_BUFFER, 1, uboLight);
      glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightingData), lightData + bufferIndex);
    }

    const GLint colorSamplerLoc = glGetUniformLocation(shaderProgram, "colorSampler");
    if (colorSamplerLoc >= 0) {
      glUniform1i(colorSamplerLoc, 0);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, tex->Id());
    }

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(indices[0]), GL_UNSIGNED_INT, reinterpret_cast<const GLvoid*>(0));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.5f, 0.3f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (colorSamplerLoc >= 0) {
      glBindTexture(GL_TEXTURE_2D, offscreen->GetTexutre());
    }
    glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(indices[0]), GL_UNSIGNED_INT, reinterpret_cast<const GLvoid*>(0));

    bufferIndex = !bufferIndex;
    window.SwapBuffers();
  }

  glDeleteProgram(shaderProgram);
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);

  return 0;
}