/**
* @file Main.cpp
*/
#include "GLFWEW.h"
#include "Texture.h"
#include "Shader.h"
#include "OffscreenBuffer.h"
#include "UniformBuffer.h"
#include "Mesh.h"
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
  { {-0.5f, -0.3f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} },
  { { 0.3f, -0.3f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f} },
  { { 0.3f,  0.5f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} },
  { {-0.5f,  0.5f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} },

  { {-0.3f,  0.3f, 0.1f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f} },
  { {-0.3f, -0.5f, 0.1f}, {0.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} },
  { { 0.5f, -0.5f, 0.1f}, {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f} },
  { { 0.5f, -0.5f, 0.1f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f} },
  { { 0.5f,  0.3f, 0.1f}, {1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f} },
  { {-0.3f,  0.3f, 0.1f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f} },

  { {-1.0f,-1.0f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f} },
  { { 1.0f,-1.0f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} },
  { { 1.0f, 1.0f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} },
  { {-1.0f, 1.0f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} },
};

/// �C���f�b�N�X�f�[�^.
const GLuint indices[] = {
  0, 1, 2, 2, 3, 0,
  4, 5, 6, 7, 8, 9,
  10, 11, 12, 12, 13, 10,
};

/// ���_�V�F�[�_�̃p�����[�^�^.
struct VertexData
{
  glm::mat4 matMVP;
};

const int maxLightCount = 4; ///< ���C�g�̐�.

/**
* ���C�g�f�[�^(�_����).
*/
struct PointLight
{
  glm::vec4 position; ///< ���W(���[���h���W�n).
  glm::vec4 color; ///< ���邳.
};

/**
* ���C�e�B���O�p�����[�^���V�F�[�_�ɓ]�����邽�߂̍\����.
*/
struct LightData
{
  glm::vec4 ambientColor; ///< ����.
  PointLight light[maxLightCount]; ///< ���C�g�̃��X�g.
};

/**
* �|�X�g�G�t�F�N�g�f�[�^���V�F�[�_�ɓ]�����邽�߂̍\����.
*/
struct PostEffectData
{
  glm::mat4x4 matColor; ///< �F�ϊ��s��.
};

/// �o�C���f�B���O�|�C���g.
enum BindingPoint
{
  BINDINGPOINT_VERTEXDATA, ///< ���_�V�F�[�_�p�p�����[�^�̃o�C���f�B���O�|�C���g.
  BINDINGPOINT_LIGHTDATA, ///< ���C�e�B���O�p�����[�^�p�̃o�C���f�B���O�|�C���g.
  BINDINGPOINT_POSTEFFECTDATA, ///< �|�X�g�G�t�F�N�g�p�����[�^�p�̃o�C���f�B���O�|�C���g.
};

/**
* �����`��f�[�^.
*/
struct RenderingPart
{
  GLvoid* offset; ///< �`��J�n�C���f�b�N�X�̃o�C�g�I�t�Z�b�g.
  GLsizei size; ///< �`�悷��C���f�b�N�X��.
};

/**
* RenderingPart���쐬����.
*
* @param offset �`��J�n�C���f�b�N�X�̃I�t�Z�b�g(�C���f�b�N�X�P��).
* @param size �`�悷��C���f�b�N�X��.
*
* @return �쐬���������`��I�u�W�F�N�g.
*/
constexpr RenderingPart MakeRenderingPart(GLsizei offset, GLsizei size) {
  return { reinterpret_cast<GLvoid*>(offset * sizeof(GLuint)), size };
}

/**
* �����`��f�[�^���X�g.
*/
static const RenderingPart renderingParts[] = {
  MakeRenderingPart(0, 12),
  MakeRenderingPart(12, 6),
};

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
* Uniform Block Object���쐬����.
*
* @param size Uniform Block�̃T�C�Y.
* @param data Uniform Block�ɓ]������f�[�^�ւ̃|�C���^.
*
* @return �쐬����UBO.
*/
GLuint CreateUBO(GLsizeiptr size, const GLvoid* data = nullptr)
{
  GLuint ubo;
  glGenBuffers(1, &ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, ubo);
  glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STREAM_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  return ubo;
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
  const UniformBufferPtr uboVertex = UniformBuffer::Create(sizeof(VertexData), BINDINGPOINT_VERTEXDATA, "VertexData");
  const UniformBufferPtr uboLight = UniformBuffer::Create(sizeof(LightData), BINDINGPOINT_LIGHTDATA, "LightData");
  const UniformBufferPtr uboPostEffect = UniformBuffer::Create(sizeof(PostEffectData), BINDINGPOINT_POSTEFFECTDATA, "PostEffectData");
  const Shader::ProgramPtr progTutorial = Shader::Program::Create("Res/Tutorial.vert", "Res/Tutorial.frag");
  const Shader::ProgramPtr progColorFilter = Shader::Program::Create("Res/Posterization.vert", "Res/Posterization.frag");
  if (!vbo || !ibo || !vao || !uboVertex || !uboLight || !uboPostEffect || !progTutorial || !progColorFilter) {
    return 1;
  }
  progTutorial->UniformBlockBinding(*uboVertex);
  progTutorial->UniformBlockBinding(*uboLight);
  progColorFilter->UniformBlockBinding(*uboPostEffect);

  // �e�N�X�`���f�[�^.
  TexturePtr tex = Texture::LoadFromFile("Res/sample.bmp");
  TexturePtr texToroid = Texture::LoadFromFile("Res/Toroid.bmp");
  if (!tex || !texToroid) {
    return 1;
  }

  Mesh::BufferPtr meshBuffer = Mesh::Buffer::Create(50000, 50000);
  meshBuffer->LoadMeshFromFile("Res/Toroid.fbx");

  const OffscreenBufferPtr offscreen = OffscreenBuffer::Create(800, 600);

  glEnable(GL_DEPTH_TEST);

  // ���C�����[�v.
  while (!window.ShouldClose()) {
    glBindFramebuffer(GL_FRAMEBUFFER, offscreen->GetFramebuffer());
    glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ���_����]�ړ�������.
    static float degree = 0.0f;
    degree += 0.1f;
    if (degree >= 360.0f) { degree -= 360.0f; }
    const glm::vec3 viewPos = glm::rotate(glm::mat4(), glm::radians(degree), glm::vec3(0, 1, 0)) * glm::vec4(2, 3, 3, 1);

    progTutorial->UseProgram();
    const glm::mat4x4 matProj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    const glm::mat4x4 matView = glm::lookAt(viewPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    VertexData vertexData;
    vertexData.matMVP = matProj * matView;
    uboVertex->BufferSubData(&vertexData);
    LightData lightData;
    lightData.ambientColor = glm::vec4(0.05f, 0.1f, 0.2f, 1);
    lightData.light[0].position = glm::vec4(1, 1, 1, 1);
    lightData.light[0].color = glm::vec4(2, 2, 2, 1);
    lightData.light[1].position = glm::vec4(-0.2f, 0, 0.6f, 1);
    lightData.light[1].color = glm::vec4(0.125f, 0.125f, 0.05f, 1);
    uboLight->BufferSubData(&lightData);

    progTutorial->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, tex->Id());

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, renderingParts[0].size, GL_UNSIGNED_INT, renderingParts[0].offset);
    progTutorial->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, texToroid->Id());
    meshBuffer->BindVAO();
    meshBuffer->GetMesh("Toroid")->Draw(meshBuffer);
    glBindVertexArray(vao);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.5f, 0.3f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    progColorFilter->UseProgram();
    progColorFilter->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offscreen->GetTexutre());

    PostEffectData postEffect;
    postEffect.matColor[0] = glm::vec4(0.393f, 0.349f, 0.272f, 0);
    postEffect.matColor[1] = glm::vec4(0.769f, 0.686f, 0.534f, 0);
    postEffect.matColor[2] = glm::vec4(0.189f, 0.168f, 0.131f, 0);
    postEffect.matColor[3] = glm::vec4(0, 0, 0, 1);
    uboPostEffect->BufferSubData(&postEffect);

    glDrawElements(GL_TRIANGLES, renderingParts[1].size, GL_UNSIGNED_INT, renderingParts[1].offset);

    window.SwapBuffers();
  }

  glDeleteVertexArrays(1, &vao);

  return 0;
}