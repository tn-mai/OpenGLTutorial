/**
* @file GameEngine.cpp
*/
#include "GameEngine.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <time.h>

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
* �|�X�g�G�t�F�N�g�p�����[�^.
*/
struct PostEffectData
{
  glm::mat4x4 matColor; ///< �F�ϊ��s��.
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
  glDeleteBuffers(1, &ibo);
  glDeleteBuffers(1, &vbo);
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
  if (vao) {
    glDeleteVertexArrays(1, &vao);
  }
}

/**
* �Q�[���G���W��������������.
*
* @retval true  ����������.
* @retval false ���������s.
*
* Update, Render�֐��Ȃǂ��Ăяo���O�ɁA��x�����Ăяo���Ă����K�v������.
* ��x�������ɐ�������ƁA�Ȍ�̌Ăяo���ł͂Ȃɂ�������true��Ԃ�.
*/
bool GameEngine::Init()
{
  if (!isInitialized) {
    if (!InitImpl()) {
      this->~GameEngine();
      new (this) GameEngine;
      return false;
    }
    isInitialized = true;
  }
  return true;
}

/**
* ��ԍX�V�֐���ݒ肷��.
*
* @param func �ݒ肷��X�V�֐�.
*
* @return �ȑO�ɐݒ肳��Ă����X�V�֐�.
*/
GameEngine::UpdateFunc GameEngine::SetUpdateFunc(const UpdateFunc& func)
{
  UpdateFunc old = updateFunc;
  updateFunc = func;
  return old;
}

/**
* �Q�[���G���W��������������.
*
* @retval true  ����������.
* @retval false ���������s.
*/
bool GameEngine::InitImpl()
{
  vbo = CreateVBO(sizeof(vertices), vertices);
  ibo = CreateIBO(sizeof(indices), indices);
  vao = CreateVAO(vbo, ibo);
  uboTrans = UniformBuffer::Create(sizeof(TransformationData), BindingPoint_Vertex, "VertexData");
  uboLight = UniformBuffer::Create(sizeof(LightingData), BindingPoint_Light, "LightingData");
  uboPostEffect = UniformBuffer::Create(sizeof(PostEffectData), 2, "PostEffectData");
  progTutorial = Shader::Program::Create("Res/Tutorial.vert", "Res/Tutorial.frag");
  progPostEffect = Shader::Program::Create("Res/PostEffect.vert", "Res/PostEffect.frag");
  progBloom1st = Shader::Program::Create("Res/Bloom1st.vert", "Res/Bloom1st.frag");
  progComposition = Shader::Program::Create("Res/FinalComposition.vert", "Res/FinalComposition.frag");
  progSimple = Shader::Program::Create("Res/Simple.vert", "Res/Simple.frag");
  progLensFlare = Shader::Program::Create("Res/AnamorphicLensFlare.vert", "Res/AnamorphicLensFlare.frag");
  if (!vbo || !ibo || !vao || !uboTrans || !uboLight || !progTutorial || !progPostEffect || !progBloom1st || !progComposition || !progLensFlare) {
    return false;
  }
  progTutorial->UniformBlockBinding("TransformationData", BindingPoint_Vertex);
  progTutorial->UniformBlockBinding("LightingData", BindingPoint_Light);
  progComposition->UniformBlockBinding("PostEffectData", 2);

  meshBuffer = Mesh::Buffer::Create(10 * 1024, 30 * 1024);
  if (!meshBuffer) {
    return false;
  }

  entityBuffer = Entity::Buffer::Create(1024, sizeof(TransformationData), BindingPoint_Vertex, "VertexData");
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

  rand.seed(time(nullptr));

  offscreen = OffscreenBuffer::Create(800, 600,  GL_RGBA16F);
  for (int i = 0, scale = 4; i < bloomBufferCount; ++i, scale *= 2) {
    const int w = (800 + scale - 1) / scale;
    const int h = (600 + scale - 1) / scale;
    offBloom[i] = OffscreenBuffer::Create(w, h, GL_RGBA8);
    if (!offBloom) {
      return false;
    }
  }
  offAnamorphic[0] = OffscreenBuffer::Create(800 / 16, 600 / 2, GL_RGBA8);
  offAnamorphic[1] = OffscreenBuffer::Create(800 / 64, 600 / 2, GL_RGBA8);
  if (!offscreen || !offAnamorphic[0] || !offAnamorphic[1]) {
    return false;
  }
  return true;
}

/**
* �Q�[���̏�Ԃ��X�V����.
*
* @param delta �O��̍X�V����̌o�ߎ���(�b).
*/
void GameEngine::Update(double delta)
{
  if (updateFunc) {
    updateFunc(delta);
  }
}

/**
* �Q�[���̏�Ԃ�`�悷��.
*/
void GameEngine::Render() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, offscreen->GetFramebuffer());
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glViewport(0, 0, 800, 600);
  glScissor(0, 0, 800, 600);
  glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
  glClearDepth(1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  const glm::mat4x4 matProj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
  const glm::mat4x4 matView = glm::lookAt(viewPos, viewTarget, viewUp);

  uboLight->BufferSubData(&lightData);

  entityBuffer->Update(1.0f / 60.0f, matView, matProj);
  entityBuffer->Draw(meshBuffer);

  glBindVertexArray(vao);

#if 0
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearColor(0.5f, 0.3f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  progTutorial->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offscreen->GetTexutre());

  TransformationData transData;
  uboTrans->BufferSubData(&transData);

  LightingData lightData;
  lightData.ambientColor = glm::vec4(1);
  uboLight->BufferSubData(&lightData);
#endif

  glDisable(GL_DEPTH_TEST);

  progBloom1st->UseProgram();
  glBindFramebuffer(GL_FRAMEBUFFER, offBloom[0]->GetFramebuffer());
  glViewport(0, 0, offBloom[0]->Width(), offBloom[0]->Height());
  progBloom1st->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offscreen->GetTexutre());
  glDrawElements(GL_TRIANGLES, renderingData[1].size, GL_UNSIGNED_INT, renderingData[1].offset);

  progPostEffect->UseProgram();
  for (int i = 1; i < bloomBufferCount; ++i) {
    glBindFramebuffer(GL_FRAMEBUFFER, offBloom[i]->GetFramebuffer());
    glViewport(0, 0, offBloom[i]->Width(), offBloom[i]->Height());
    progPostEffect->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offBloom[i - 1]->GetTexutre());
    glDrawElements(GL_TRIANGLES, renderingData[1].size, GL_UNSIGNED_INT, renderingData[1].offset);
  }

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
  glBlendFunc(GL_ONE, GL_ONE);
  progSimple->UseProgram();

  glBindFramebuffer(GL_FRAMEBUFFER, offAnamorphic[0]->GetFramebuffer());
  glViewport(0, 0, offAnamorphic[0]->Width(), offAnamorphic[0]->Height());
  progSimple->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offAnamorphic[1]->GetTexutre());
  glDrawElements(GL_TRIANGLES, renderingData[1].size, GL_UNSIGNED_INT, renderingData[1].offset);
  glDrawElements(GL_TRIANGLES, renderingData[2].size, GL_UNSIGNED_INT, renderingData[2].offset);
  glDrawElements(GL_TRIANGLES, renderingData[3].size, GL_UNSIGNED_INT, renderingData[3].offset);

  for (int i = bloomBufferCount - 1; i > 0; --i) {
    glBindFramebuffer(GL_FRAMEBUFFER, offBloom[i - 1]->GetFramebuffer());
    glViewport(0, 0, offBloom[i - 1]->Width(), offBloom[i - 1]->Height());
    progSimple->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offBloom[i]->GetTexutre());
    glDrawElements(GL_TRIANGLES, renderingData[1].size, GL_UNSIGNED_INT, renderingData[1].offset);
  }

  glDisable(GL_BLEND);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, 800, 600);
  progComposition->UseProgram();

  PostEffectData postEffect;
#if 0
  postEffect.matColor[0] = glm::vec4(0.393f, 0.349f, 0.272f, 0);
  postEffect.matColor[1] = glm::vec4(0.769f, 0.686f, 0.534f, 0);
  postEffect.matColor[2] = glm::vec4(0.189f, 0.168f, 0.131f, 0);
  postEffect.matColor[3] = glm::vec4(0, 0, 0, 1);
#elif 0
  postEffect.matColor[0] = glm::vec4(-1, 0, 0, 0);
  postEffect.matColor[1] = glm::vec4(0, -1, 0, 0);
  postEffect.matColor[2] = glm::vec4(0, 0, -1, 0);
  postEffect.matColor[3] = glm::vec4(1, 1, 1, 1);
#endif
  uboPostEffect->BufferSubData(&postEffect);

  progComposition->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offscreen->GetTexutre());
  progComposition->BindTexture(GL_TEXTURE1, GL_TEXTURE_2D, offBloom[0]->GetTexutre());
  progComposition->BindTexture(GL_TEXTURE2, GL_TEXTURE_2D, offAnamorphic[0]->GetTexutre());
  glDrawElements(GL_TRIANGLES, renderingData[1].size, GL_UNSIGNED_INT, renderingData[1].offset);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

/**
*
*/
bool GameEngine::LoadTextureFromFile(const char* filename)
{
  const auto itr = textureBuffer.find(filename);
  if (itr != textureBuffer.end()) {
    return true;
  }
  TexturePtr texture = Texture::LoadFromFile(filename);
  if (!texture) {
    return false;
  }
  textureBuffer.insert(std::make_pair(std::string(filename), texture));
  return true;
}

/*
*
*/
const TexturePtr& GameEngine::GetTexture(const char* filename) const
{
  const auto itr = textureBuffer.find(filename);
  if (itr != textureBuffer.end()) {
    return itr->second;
  }
  static const TexturePtr dummy;
  return dummy;
}

/**
*
*/
bool GameEngine::LoadMeshFromFile(const char* filename)
{
  return meshBuffer->LoadMeshFromFile(filename);
}

/**
*
*/
const Mesh::MeshPtr& GameEngine::GetMesh(const char* name)
{
  return meshBuffer->GetMesh(name);
}

/**
*
*/
Entity::Entity* GameEngine::AddEntity(const glm::vec3& pos, const char* meshName, const char* texName, Entity::Entity::UpdateFuncType func)
{
  const Mesh::MeshPtr& mesh = meshBuffer->GetMesh(meshName);
  const TexturePtr& tex = GetTexture(texName);
  return entityBuffer->AddEntity(pos, mesh, tex, progTutorial, func);
}

/**
*
*/
void GameEngine::RemoveEntity(Entity::Entity* e)
{
  entityBuffer->RemoveEntity(e);
}

/**
* �f�t�H���g��VertexData�X�V�֐�.
*/
void DefaultUpdateVertexData(Entity::Entity& e, void* ubo, double, const glm::mat4& matView, const glm::mat4& matProj)
{
  GameEngine::TransformationData data;
  data.matModel = e.TRSMatrix();
  data.matNormal = glm::mat4_cast(e.Rotation());
  data.matMVP = matProj * matView * data.matModel;
  memcpy(ubo, &data, sizeof(data));
}