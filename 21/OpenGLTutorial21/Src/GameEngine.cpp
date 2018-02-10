/**
* @file GameEngine.cpp
*/
#include "GameEngine.h"
#include "GLFWEW.h"
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <iostream>
#include <time.h>

#include "Audio.h"

/// ���_�f�[�^�^.
struct Vertex
{
  glm::vec3 position; ///< ���W
  glm::vec4 color; ///< �F
  glm::vec2 texCoord; ///< �e�N�X�`�����W.
};

/// ���_�f�[�^.
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

  { {-1.0f,-1.0f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, { 0.0f, 0.0f} },
  { { 1.0f,-1.0f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, { 1.0f, 0.0f} },
  { { 1.0f, 1.0f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, { 1.0f, 1.0f} },
  { {-1.0f, 1.0f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, { 0.0f, 1.0f} },

  { {-1.25f,-1.0f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, { 0.0f, 0.0f} },
  { { 0.75f,-1.0f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, { 1.0f, 0.0f} },
  { { 0.75f, 1.0f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, { 1.0f, 1.0f} },
  { {-1.25f, 1.0f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, { 0.0f, 1.0f} },

  { {-0.75f,-1.0f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, { 0.0f, 0.0f} },
  { { 1.25f,-1.0f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, { 1.0f, 0.0f} },
  { { 1.25f, 1.0f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, { 1.0f, 1.0f} },
  { {-0.75f, 1.0f, 0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, { 0.0f, 1.0f} },
};

/// �C���f�b�N�X�f�[�^.
const GLuint indices[] = {
  0, 1, 2, 2, 3, 0,
  4, 5, 6, 7, 8, 9,
  10, 11, 12, 12, 13, 10,
  14, 15, 16, 16, 17, 14,
  18, 19, 20, 20, 21, 18,
};

/**
* �C���f�b�N�X�`��͈�.
*/
struct RenderingData
{
  GLsizei size; ///< �`�悷��C���f�b�N�X��.
  GLvoid* offset; ///< �`��J�n�C���f�b�N�X�̃o�C�g�I�t�Z�b�g.
};

/**
* RenderingData���쐬����.
*
* @param size �`�悷��C���f�b�N�X��.
* @param offset �`��J�n�C���f�b�N�X�̃I�t�Z�b�g(�C���f�b�N�X�P��).
*
* @return �C���f�b�N�X�`��͈̓I�u�W�F�N�g.
*/
constexpr RenderingData MakeRenderingData(GLsizei size, GLsizei offset) {
  return { size, reinterpret_cast<GLvoid*>(offset * sizeof(GLuint)) };
}

/**
* �`��͈̓��X�g.
*/
static const RenderingData renderingData[] = {
  MakeRenderingData( 3 * 4, 0),
  MakeRenderingData( 3 * 2, 12),
  MakeRenderingData( 3 * 2, 18),
  MakeRenderingData( 3 * 2, 24),
};

/**
* Uniform Buffer�̃o�C���f�B���O�E�|�C���g.
*/
enum BindingPoint {
  BindingPoint_Vertex,
  BindingPoint_Light,

  countof_BindingPoint, ///< ��`�����o�C���f�B���O�E�|�C���g�̐�.
};

/**
* Vertex Buffer Object���쐬����.
*
* @param size ���_�f�[�^�̃T�C�Y.
* @param data ���_�f�[�^�ւ̃|�C���^.
*
* @return �쐬����VBO��ID.
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
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STREAM_DRAW);
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
#define SetVertexAttribPointer(index, cls, mbr) \
  SetVertexAttribPointerI(index, sizeof(cls::mbr) / sizeof(float), sizeof(cls), reinterpret_cast<GLvoid*>(offsetof(cls, mbr)))
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
  SetVertexAttribPointer(0, Vertex, position);
  SetVertexAttribPointer(1, Vertex, color);
  SetVertexAttribPointer(2, Vertex, texCoord);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBindVertexArray(0);
  return vao;
}

/**
* �Q�[���G���W���̃C���X�^���X���擾����.
*
* @return �Q�[���G���W���̃C���X�^���X.
*/
GameEngine& GameEngine::Instance() {
  static GameEngine instance;
  return instance;
}

/**
* �f�X�g���N�^.
*/
GameEngine::~GameEngine()
{
  updateFunc = nullptr;
  Audio::Destroy();
  if (vao) {
    glDeleteVertexArrays(1, &vao);
  }
  if (ibo) {
    glDeleteBuffers(1, &ibo);
  }
  if (vbo) {
    glDeleteBuffers(1, &vbo);
  }
}

/**
* ��ԍX�V�֐���ݒ肷��.
*
* @param func �ݒ肷��X�V�֐�.
*/
void GameEngine::UpdateFunc(const UpdateFuncType& func)
{
  updateFunc = func;
}

/**
* ��ԍX�V�֐����擾����.
*
* @return �ݒ肳��Ă���X�V�֐�.
*/
const GameEngine::UpdateFuncType& GameEngine::UpdateFunc() const
{
  return updateFunc;
}

/**
* �Q�[���G���W��������������.
*
* @param w     �E�B���h�E�̕`��͈͂̕�(�s�N�Z��).
* @param h     �E�B���h�E�̕`��͈͂̍���(�s�N�Z��).
* @param title �E�B���h�E�^�C�g��(UTF-8��0�I�[������).
*
* @retval true  ����������.
* @retval false ���������s.
*
* Update, Render�֐��Ȃǂ��Ăяo���O�ɁA��x�����Ăяo���Ă����K�v������.
* ��x�������ɐ�������ƁA�Ȍ�̌Ăяo���ł͂Ȃɂ�������true��Ԃ�.
*/
bool GameEngine::Init(int w, int h, const char* title)
{
  if (isInitialized) {
    return true;
  }
  if (!GLFWEW::Window::Instance().Init(w, h, title)) {
    return false;
  }
  vbo = CreateVBO(sizeof(vertices), vertices);
  ibo = CreateIBO(sizeof(indices), indices);
  vao = CreateVAO(vbo, ibo);

  uboLight = UniformBuffer::Create(sizeof(Uniform::LightingData), BindingPoint_Light, "LightingData");
  uboPostEffect = UniformBuffer::Create(sizeof(Uniform::PostEffectData), 2, "PostEffectData");
  if (!vbo || !ibo || !vao || !uboLight || !uboPostEffect) {
    return false;
  }

  static const char* const shaderNameList[][3] = {
    { "Tutorial", "Res/Tutorial.vert", "Res/Tutorial.frag" },
    { "PostEffect", "Res/PostEffect.vert", "Res/PostEffect.frag" },
    { "Bloom", "Res/Bloom1st.vert", "Res/Bloom1st.frag" },
    { "Composition", "Res/FinalComposition.vert", "Res/FinalComposition.frag" },
    { "Simple", "Res/Simple.vert", "Res/Simple.frag" },
    { "LensFlare", "Res/AnamorphicLensFlare.vert", "Res/AnamorphicLensFlare.frag" },
    { "NonLighting", "Res/NonLighting.vert", "Res/NonLighting.frag" },
    { "RenderDepth", "Res/RenderDepth.vert", "Res/RenderDepth.frag" },
  };
  shaderMap.reserve(sizeof(shaderNameList) / sizeof(shaderNameList[0]));
  for (auto& e : shaderNameList) {
    Shader::ProgramPtr program = Shader::Program::Create(e[1], e[2]);
    if (!program) {
      return false;
    }
    shaderMap.insert(std::make_pair(std::string(e[0]), program));
  }
  shaderMap["Tutorial"]->UniformBlockBinding("VertexData", BindingPoint_Vertex);
  shaderMap["Tutorial"]->UniformBlockBinding("LightingData", BindingPoint_Light);
  shaderMap["Composition"]->UniformBlockBinding("PostEffectData", 2);
  shaderMap["Bloom"]->UniformBlockBinding("PostEffectData", 2);

  meshBuffer = Mesh::Buffer::Create(60 * 1024, 60 * 1024);
  if (!meshBuffer) {
    return false;
  }
  textureMapStack.push_back(TextureMap());

  entityBuffer = Entity::Buffer::Create(1024, sizeof(Uniform::VertexData), BindingPoint_Vertex, "VertexData");
  if (!entityBuffer) {
    return false;
  }

  static const uint32_t textureData[] = {
    0xffffffff, 0xffcccccc, 0xffffffff, 0xffcccccc, 0xffffffff,
    0xff888888, 0xffffffff, 0xff888888, 0xffffffff, 0xff888888,
    0xffffffff, 0xff444444, 0xffffffff, 0xff444444, 0xffffffff,
    0xff000000, 0xffffffff, 0xff000000, 0xffffffff, 0xff000000,
    0xffffffff, 0xff000000, 0xffffffff, 0xff000000, 0xffffffff,
  };
  //  TexturePtr tex = Texture::Create(5, 5, GL_RGBA8, GL_RGBA, textureData);

  rand.seed(std::random_device()());

  offscreen = OffscreenBuffer::Create(800, 600,  GL_RGBA16F);
  for (int i = 0, scale = 4; i < bloomBufferCount; ++i, scale *= 4) {
    const int w = (800 + scale - 1) / scale;
    const int h = (600 + scale - 1) / scale;
    offBloom[i] = OffscreenBuffer::Create(w, h, GL_RGBA16F);
    if (!offBloom) {
      return false;
    }
  }
  offAnamorphic[0] = OffscreenBuffer::Create(800 / 16, 600 / 2, GL_RGBA8);
  offAnamorphic[1] = OffscreenBuffer::Create(800 / 64, 600 / 2, GL_RGBA8);
  if (!offscreen || !offAnamorphic[0] || !offAnamorphic[1]) {
    return false;
  }

  {
    int width, height;
    glBindTexture(GL_TEXTURE_2D, offBloom[bloomBufferCount - 1]->GetTexutre());
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    glBindTexture(GL_TEXTURE_2D, 0);
    for (auto& e : pbo) {
      e.Init(GL_PIXEL_PACK_BUFFER, width * height * sizeof(GLfloat) * 4, nullptr, GL_DYNAMIC_READ);
    }
  }

  offDepth = OffscreenBuffer::Create(2048, 2048, GL_DEPTH_COMPONENT16);
  if (!offDepth) {
    return false;
  }

  fontRenderer.Init(1024, glm::vec2(800, 600));

  camera[0].isActive = true;

  isInitialized = true;
  return true;
}

/**
* �Q�[���̏�Ԃ��X�V����.
*
* @param delta �O��̍X�V����̌o�ߎ���(�b).
*/
void GameEngine::Update(double delta)
{
  Audio::Update();
  fontRenderer.MapBuffer();
  if (updateFunc) {
    updateFunc(delta);
  }
  const GLFWEW::Window& window = GLFWEW::Window::Instance();
  const glm::mat4x4 matProj = glm::perspective(glm::radians(45.0f), static_cast<float>(window.Width()) / static_cast<float>(window.Height()), 1.0f, 1000.0f);
  glm::mat4x4 matView[Uniform::maxViewCount];
  for (int i = 0; i < Uniform::maxViewCount; ++i) {
    if (!camera[i].isActive) {
      continue;
    }
    const CameraData& cam = camera[i].camera;
    matView[i] = glm::lookAt(cam.position, cam.target, cam.up);
  }
  const glm::vec2 range = shadowParameter.range * 0.5f;
  glm::mat4 depthProjectionMatrix = glm::ortho<float>(-range.x, range.x, -range.y, range.y, shadowParameter.near, shadowParameter.far);
  glm::mat4 depthViewMatrix = glm::lookAt(shadowParameter.lightPos, shadowParameter.lightPos + shadowParameter.lightDir, shadowParameter.lightUp);
  glm::mat4 depthMVP = depthProjectionMatrix * depthViewMatrix;

  entityBuffer->Update(delta, matView, matProj, depthMVP);
  fontRenderer.UnmapBuffer();
}

struct GameEngine::RenderingContext
{
  std::array<int, Uniform::maxViewCount> cameraIndices;
};

/**
* �D�揇�Ń\�[�g���ꂽ�J�����C���f�b�N�X�z����쐬����.
*/
void GameEngine::InitRenderingContext(RenderingContext& context) const
{
  for (int i = 0; i < Uniform::maxViewCount; ++i) {
    context.cameraIndices[i] = i;
  }
  std::stable_sort(context.cameraIndices.begin(), context.cameraIndices.end(), [&](int lhs, int rhs) {
    return camera[lhs].priority > camera[rhs].priority;
  });
}

/**
* �e��`�悷��.
*/
void GameEngine::RenderShadow(RenderingContext& context) const
{
  glBindFramebuffer(GL_FRAMEBUFFER, offDepth->GetFramebuffer());
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);
  glDisable(GL_BLEND);
  glViewport(0, 0, offDepth->Width(), offDepth->Height());
  glScissor(0, 0, offDepth->Width(), offDepth->Height());
  glClearDepth(1);
  glClear(GL_DEPTH_BUFFER_BIT);

  const Shader::ProgramPtr& progDepth = shaderMap.find("RenderDepth")->second;
  progDepth->UseProgram();

  for (int index : context.cameraIndices) {
    if (camera[index].isActive) {
      entityBuffer->DrawDepth(index, meshBuffer);
    }
  }
}

/**
* �Q�[���̏�Ԃ�`�悷��.
*/
void GameEngine::Render() const
{
  RenderingContext context;
  InitRenderingContext(context);

  RenderShadow(context);

  glBindFramebuffer(GL_FRAMEBUFFER, offscreen->GetFramebuffer());
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_CULL_FACE);
  glViewport(0, 0, offscreen->Width(), offscreen->Height());
  glScissor(0, 0, offscreen->Width(), offscreen->Height());
  glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
  glClearDepth(1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_BLEND);
  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

  shaderMap.find("Tutorial")->second->BindShadowTexture(GL_TEXTURE_2D, offDepth->GetTexutre());
  uboLight->BufferSubData(&lightData);
  for (int index : context.cameraIndices) {
    if (camera[index].isActive) {
      entityBuffer->Draw(index, meshBuffer);
    }
  }
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, 0);

  glBindVertexArray(vao);

#if 0
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearColor(0.5f, 0.3f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  progTutorial->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offscreen->GetTexutre());

  VertexData transData;
  uboTrans->BufferSubData(&transData);

  LightingData lightData;
  lightData.ambientColor = glm::vec4(1);
  uboLight->BufferSubData(&lightData);
#endif

  glDisable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glDisable(GL_CULL_FACE);
  glDisable(GL_BLEND);

  Uniform::PostEffectData postEffect;
#if 0
  postEffect.matColor[0] = glm::vec4(0.393f, 0.349f, 0.272f, 0);
  postEffect.matColor[1] = glm::vec4(0.769f, 0.686f, 0.534f, 0);
  postEffect.matColor[2] = glm::vec4(0.189f, 0.168f, 0.131f, 0);
  postEffect.matColor[3] = glm::vec4(0, 0, 0, 1);
#elif 0
  static float x = 1, xx = -0.01f;
  postEffect.matColor[0] = glm::vec4(x, 0, 0, 0);
  postEffect.matColor[1] = glm::vec4(0, x, 0, 0);
  postEffect.matColor[2] = glm::vec4(0, 0, x, 0);
  postEffect.matColor[3] = glm::vec4(0, 0, 0, 1);
  x += xx;
  if (x < -1.0f) {
    xx = 0.01f;
  } else if (x >= 1.0f) {
    xx = -0.01f;
  }
#endif
  postEffect.lumScale = luminanceScale;
  postEffect.bloomThreshould = 1.0f / luminanceScale;
  uboPostEffect->BufferSubData(&postEffect);

  const Shader::ProgramPtr& progBloom = shaderMap.find("Bloom")->second;
  progBloom->UseProgram();
  glBindFramebuffer(GL_FRAMEBUFFER, offBloom[0]->GetFramebuffer());
  glViewport(0, 0, offBloom[0]->Width(), offBloom[0]->Height());
  progBloom->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offscreen->GetTexutre());
  glDrawElements(GL_TRIANGLES, renderingData[1].size, GL_UNSIGNED_INT, renderingData[1].offset);

  const Shader::ProgramPtr& progShrink = shaderMap.find("Simple")->second;
  progShrink->UseProgram();
  for (int i = 1; i < bloomBufferCount; ++i) {
    glBindFramebuffer(GL_FRAMEBUFFER, offBloom[i]->GetFramebuffer());
    glViewport(0, 0, offBloom[i]->Width(), offBloom[i]->Height());
    progShrink->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offBloom[i - 1]->GetTexutre());
    glDrawElements(GL_TRIANGLES, renderingData[1].size, GL_UNSIGNED_INT, renderingData[1].offset);
  }

  const Shader::ProgramPtr& progLensFlare = shaderMap.find("LensFlare")->second;
  progLensFlare->UseProgram();
  glBindFramebuffer(GL_FRAMEBUFFER, offAnamorphic[0]->GetFramebuffer());
  glViewport(0, 0, offAnamorphic[0]->Width(), offAnamorphic[0]->Height());
  progLensFlare->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offBloom[0]->GetTexutre());
  glDrawElements(GL_TRIANGLES, renderingData[1].size, GL_UNSIGNED_INT, renderingData[1].offset);

  glBindFramebuffer(GL_FRAMEBUFFER, offAnamorphic[1]->GetFramebuffer());
  glViewport(0, 0, offAnamorphic[1]->Width(), offAnamorphic[1]->Height());
  progLensFlare->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offAnamorphic[0]->GetTexutre());
  glDrawElements(GL_TRIANGLES, renderingData[1].size, GL_UNSIGNED_INT, renderingData[1].offset);

  glEnable(GL_BLEND);
  glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);

  const Shader::ProgramPtr& progEnlarge = shaderMap.find("PostEffect")->second;
  progEnlarge->UseProgram();
  glBindFramebuffer(GL_FRAMEBUFFER, offAnamorphic[0]->GetFramebuffer());
  glViewport(0, 0, offAnamorphic[0]->Width(), offAnamorphic[0]->Height());
  progEnlarge->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offAnamorphic[1]->GetTexutre());
  glDrawElements(GL_TRIANGLES, renderingData[1].size, GL_UNSIGNED_INT, renderingData[1].offset);
  glDrawElements(GL_TRIANGLES, renderingData[2].size, GL_UNSIGNED_INT, renderingData[2].offset);
  glDrawElements(GL_TRIANGLES, renderingData[3].size, GL_UNSIGNED_INT, renderingData[3].offset);

  for (int i = bloomBufferCount - 1; i > 0; --i) {
    glBindFramebuffer(GL_FRAMEBUFFER, offBloom[i - 1]->GetFramebuffer());
    glViewport(0, 0, offBloom[i - 1]->Width(), offBloom[i - 1]->Height());
    progEnlarge->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offBloom[i]->GetTexutre());
    glDrawElements(GL_TRIANGLES, renderingData[1].size, GL_UNSIGNED_INT, renderingData[1].offset);
  }

  glDisable(GL_BLEND);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  GLFWEW::Window& window = GLFWEW::Window::Instance();
  const int windowWidth = window.Width();
  const int windowHeight = window.Height();
  glViewport(0, 0, windowWidth, windowHeight);
  glScissor(0, 0, windowWidth, windowHeight);

  const Shader::ProgramPtr& progComposition = shaderMap.find("Composition")->second;
  progComposition->UseProgram();

  progComposition->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offscreen->GetTexutre());
  progComposition->BindTexture(GL_TEXTURE1, GL_TEXTURE_2D, offBloom[0]->GetTexutre());
  progComposition->BindTexture(GL_TEXTURE2, GL_TEXTURE_2D, offAnamorphic[0]->GetTexutre());
  glDrawElements(GL_TRIANGLES, renderingData[1].size, GL_UNSIGNED_INT, renderingData[1].offset);

  fontRenderer.Draw();

  {
    int width, height;
    glBindTexture(GL_TEXTURE_2D, offBloom[bloomBufferCount - 1]->GetTexutre());
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (pboIndexForWriting < 0) {
      const GLuint pboWrite = pbo[1].Id();
      glBindBuffer(GL_PIXEL_PACK_BUFFER, pboWrite);
      glBindFramebuffer(GL_FRAMEBUFFER, offBloom[bloomBufferCount - 1]->GetFramebuffer());
      glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, 0);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    } else {
      const GLuint pboWrite = pbo[pboIndexForWriting].Id();
      glBindBuffer(GL_PIXEL_PACK_BUFFER, pboWrite);
      glBindFramebuffer(GL_FRAMEBUFFER, offBloom[bloomBufferCount - 1]->GetFramebuffer());
      glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, 0);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      const GLuint pboRead = pbo[pboIndexForWriting ^ 1].Id();
      glBindBuffer(GL_PIXEL_PACK_BUFFER, pboRead);
      const GLfloat* p = static_cast<GLfloat*>(glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY));
      float lum = 0;
      for (int i = 0; i < width * height; ++i) {
        lum += p[i * 4 + 3];
      }
      luminanceScale = keyValue / std::exp(lum / static_cast<float>(width * height));
      glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
      glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    }
  }

  glBindVertexArray(0);
  glUseProgram(0);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

/**
* �Q�[�������s����.
*/
void GameEngine::Run()
{
  GLFWEW::Window& window = GLFWEW::Window::Instance();
  double prevTime = glfwGetTime();
  double frames = 0;
  double fpsTimer = 0;
  while (!window.ShouldClose()) {
    const double curTime = glfwGetTime();
    const double delta = curTime - prevTime;
    prevTime = curTime;
    frames += 1;
    fpsTimer += delta;
    if (fpsTimer >= 1) {
      fpsTimer -= 1;
      fps = frames;
      frames = 0;
    }
    window.UpdateGamePad();
    Update(delta <= 0.5 ? delta : 1.0 / 60.0);
    Render();
    window.SwapBuffers();
    if (window.GetKey(GLFW_KEY_ESCAPE) == GLFWEW::Window::KeyState::Press) {
      window.Close();
    }
    if (pboIndexForWriting < 0) {
      pboIndexForWriting = 0;
    } else {
      pboIndexForWriting ^= 1;
    }
  }
}

/**
* �Q�[���p�b�h�̏�Ԃ��擾����.
*/
const GamePad& GameEngine::GetGamePad(int id) const
{
  return GLFWEW::Window::Instance().GetGamePad(id);
}

/**
* �e�N�X�`����ǂݍ���.
*
* @param filename �e�N�X�`���t�@�C����.
*
* @retval true  �ǂݍ��ݐ���.
* @retval false �ǂݍ��ݎ��s.
*/
bool GameEngine::LoadTextureFromFile(const char* filename, GLenum wrapMode)
{
  if (GetTexture(filename)) {
    return true;
  }
  TexturePtr texture = Texture::LoadFromFile(filename, wrapMode);
  if (!texture) {
    return false;
  }
  textureMapStack.back().insert(std::make_pair(std::string(filename), texture));
  return true;
}

/*
* �e�N�X�`�����擾����.
*
* @param filename �e�N�X�`���t�@�C����.
*
*
* @return filename�ɑΉ�����e�N�X�`���I�u�W�F�N�g.
*/
const TexturePtr& GameEngine::GetTexture(const char* filename) const
{
  for (const auto& e : textureMapStack) {
    const auto itr = e.find(filename);
    if (itr != e.end()) {
      return itr->second;
    }
  }
  static const TexturePtr dummy;
  return dummy;
}

/**
* ���b�V����ǂݍ���.
*
* @param filename ���b�V���t�@�C����.
*
* @retval true  �ǂݍ��ݐ���.
* @retval false �ǂݍ��ݎ��s.
*/
bool GameEngine::LoadMeshFromFile(const char* filename)
{
  return meshBuffer->LoadMeshFromFile(filename);
}

/**
* ���b�V�����擾����.
*
* @param name ���b�V����.
*
* @return name�ɑΉ����郁�b�V���I�u�W�F�N�g.
*/
const Mesh::MeshPtr& GameEngine::GetMesh(const char* name)
{
  return meshBuffer->GetMesh(name);
}

/**
* �G���e�B�e�B��ǉ�����.
*
* @param groupId  �G���e�B�e�B�̃O���[�vID.
* @param position �G���e�B�e�B�̍��W.
* @param meshName �G���e�B�e�B�̕\���Ɏg�p���郁�b�V����.
* @param texName  �G���e�B�e�B�̕\���Ɏg���e�N�X�`���t�@�C����.
* @param func     �G���e�B�e�B�̏�Ԃ��X�V����֐�(�܂��͊֐��I�u�W�F�N�g).
* @param shader   �G���e�B�e�B�̕\���Ɏg���V�F�[�_��.
*
* @return �ǉ������G���e�B�e�B�ւ̃|�C���^.
*         ����ȏ�G���e�B�e�B��ǉ��ł��Ȃ��ꍇ��nullptr���Ԃ����.
*         ��]��g�嗦�͂��̃|�C���^�o�R�Őݒ肷��.
*         �Ȃ��A���̃|�C���^���A�v���P�[�V�������ŕێ�����K�v�͂Ȃ�.
*/
Entity::Entity* GameEngine::AddEntity(int groupId, const glm::vec3& pos, const char* meshName, const char* texName, Entity::Entity::UpdateFuncType func, const char* shader)
{
  return AddEntity(groupId, pos, meshName, texName, nullptr, func, shader);
}

Entity::Entity* GameEngine::AddEntity(int groupId, const glm::vec3& pos, const char* meshName, const char* texName, const char* normalName, Entity::Entity::UpdateFuncType func, const char* shader)
{
  decltype(shaderMap)::const_iterator itr = shaderMap.end();
  if (shader) {
    itr = shaderMap.find(shader);
  }
  if (itr == shaderMap.end()) {
    itr = shaderMap.find("Tutorial");
    if (itr == shaderMap.end()) {
      return nullptr;
    }
  }
  const Mesh::MeshPtr& mesh = meshBuffer->GetMesh(meshName);
  TexturePtr tex[2];
  tex[0] = GetTexture(texName);
  if (normalName) {
    tex[1] = GetTexture(normalName);
  } else {
    tex[1] = GetTexture("Res/Model/Dummy.Normal.bmp");
  }
  return entityBuffer->AddEntity(groupId, pos, mesh, tex, itr->second, func);
}

/**
*�@�G���e�B�e�B���폜����.
*
* @param �폜����G���e�B�e�B�̃|�C���^.
*/
void GameEngine::RemoveEntity(Entity::Entity* e)
{
  entityBuffer->RemoveEntity(e);
}

/**
* �S�ẴG���e�B�e�B���폜����.
*/
void GameEngine::RemoveAllEntity()
{
  entityBuffer->RemoveAllEntity();
}

/**
* ���C�g��ݒ肷��.
*
* @param indes  �ݒ肷�郉�C�g�̃C���f�b�N�X.
* @param light  ���C�g�p�����[�^.
*/
void GameEngine::Light(int index, const Uniform::PointLight& light)
{
  if (index < 0 || index >= Uniform::maxLightCount) {
    std::cerr << "WARNING: '" << index << "'�͕s���ȃ��C�g�C���f�b�N�X�ł�" << std::endl;
    return;
  }
  lightData.light[index] = light;
}

/**
* ���C�g���擾����.
*
* @param index �擾���郉�C�g�̃C���f�b�N�X.
*
* @return ���C�g�p�����[�^.
*/
const Uniform::PointLight& GameEngine::Light(int index) const
{
  if (index < 0 || index >= Uniform::maxLightCount) {
    std::cerr << "WARNING: '" << index << "'�͕s���ȃ��C�g�C���f�b�N�X�ł�" << std::endl;
    static const Uniform::PointLight dummy;
    return dummy;
  }
  return lightData.light[index];
}

/**
* ������ݒ肷��.
*
* @param color �����̖��邳.
*/
void GameEngine::AmbientLight(const glm::vec4& color)
{
  lightData.ambientColor = color;
}

/**
* �������擾����.
*
* @return �����̖��邳.
*/
const glm::vec4& GameEngine::AmbientLight() const
{
  return lightData.ambientColor;
}

/**
* ���_�̈ʒu�Ǝp����ݒ肷��.
*
* @param index �J�����̃C���f�b�N�X.
* @param cam   �ݒ肷��J�����f�[�^.
*
* �w�肳�ꂽ�C���f�b�N�X�̃J�������A�N�e�B�u�����A��Ԃ�ݒ肷��.
*
* @sa Camera, ActivateCamera, DeactivateCamera, IsCameraActive
*/
void GameEngine::Camera(size_t index, const CameraData& cam)
{
  camera[index].camera = cam;
  camera[index].isActive = true;
  lightData.eyePos[index] = glm::vec4(cam.position, 0);
}

/**
* ���_�̈ʒu�Ǝp�����擾����.
*
* @param index �J�����̃C���f�b�N�X.
* @return �J�����f�[�^.
*/
const GameEngine::CameraData& GameEngine::Camera(size_t index) const
{
  return camera[index].camera;
}

/**
* �S�ẴJ�����̏�Ԃ�������Ԃɖ߂�.
*/
void GameEngine::ResetAllCamera()
{
  for (auto& c : camera) {
    c.camera = {};
    c.isActive = false;
    c.priority = 0;
  }
  camera[0].isActive = true;
}

/**
* �����I�u�W�F�N�g���擾����.
*
* @return �����I�u�W�F�N�g.
*/
std::mt19937& GameEngine::Rand()
{
  return rand;
}

/**
* �Փˉ����n���h����ݒ肷��.
*
* @param gid0    �ՓˑΏۂ̃O���[�vID.
* @param gid1    �ՓˑΏۂ̃O���[�vID.
* @param handler �Փˉ����n���h��.
*
* �Փ˂��������Փˉ����n���h�����Ăт����Ƃ��A��菬�����O���[�vID�����G���e�B�e�B�����ɓn�����.
* �����Ŏw�肵���O���[�vID�̏����Ƃ͖��֌W�ł��邱�Ƃɒ��ӂ��邱��.
* ex)
*   CollisionHandler(10, 1, Func)
*   �Ƃ����R�[�h�Ńn���h����o�^�����Ƃ���. �Փ˂���������ƁA
*   Func(�O���[�vID=1�̃G���e�B�e�B�A�O���[�vID=10�̃G���e�B�e�B)
*   �̂悤�ɌĂяo�����.
*/
void GameEngine::CollisionHandler(int gid0, int gid1, Entity::CollisionHandlerType handler)
{
  entityBuffer->CollisionHandler(gid0, gid1, handler);
}

/**
* �Փˉ����n���h�����擾����.
*
* @param gid0 �ՓˑΏۂ̃O���[�vID.
* @param gid1 �ՓˑΏۂ̃O���[�vID.
*
* @return �Փˉ����n���h��.
*/
const Entity::CollisionHandlerType& GameEngine::CollisionHandler(int gid0, int gid1) const
{
  return entityBuffer->CollisionHandler(gid0, gid1);
}

/**
* �Փˉ����n���h���̃��X�g���N���A����.
*/
void GameEngine::ClearCollisionHandlerList()
{
  entityBuffer->ClearCollisionHandlerList();
}

/**
* �I�[�f�B�I������������.
*/
bool GameEngine::InitAudio(const char* acfPath, const char* acbPath, const char* awbPath, const char* dspBusName)
{
  return Audio::Initialize(acfPath, acbPath, awbPath, dspBusName);
}

/**
* �����Đ�����.
*/
void GameEngine::PlayAudio(int playerId, int cueId)
{
  Audio::Play(playerId, cueId);
}

/**
* �����~����.
*/
void GameEngine::StopAudio(int playerId)
{
  Audio::Stop(playerId);
}

/**
* ���\�[�X�X�^�b�N�ɐV�������\�[�X���x�����쐬����.
*/
void GameEngine::PushLevel()
{
  meshBuffer->PushLevel();
  textureMapStack.push_back(TextureMap());
}

/**
* ���\�[�X�X�^�b�N�̖����̃��\�[�X���x������������.
*/
void GameEngine::PopLevel()
{
  meshBuffer->PopLevel();
  if (textureMapStack.size() > minimalStackSize) {
    textureMapStack.pop_back();
  }
}

/**
* �����̃��\�[�X���x������̏�Ԃɂ���.
*/
void GameEngine::ClearLevel()
{
  meshBuffer->ClearLevel();
  textureMapStack.back().clear();
}