/**
* @file GameEngine.cpp
*/
#include "GameEngine.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <time.h>

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

/// インデックスデータ.
const GLuint indices[] = {
  0, 1, 2, 2, 3, 0,
  4, 5, 6, 7, 8, 9,
  10, 11, 12, 12, 13, 10,
  14, 15, 16, 16, 17, 14,
  18, 19, 20, 20, 21, 18,
};

/**
* インデックス描画範囲.
*/
struct RenderingData
{
  GLsizei size; ///< 描画するインデックス数.
  GLvoid* offset; ///< 描画開始インデックスのバイトオフセット.
};

/**
* RenderingDataを作成する.
*
* @param size 描画するインデックス数.
* @param offset 描画開始インデックスのオフセット(インデックス単位).
*
* @return インデックス描画範囲オブジェクト.
*/
constexpr RenderingData MakeRenderingData(GLsizei size, GLsizei offset) {
  return { size, reinterpret_cast<GLvoid*>(offset * sizeof(GLuint)) };
}

/**
* 描画範囲リスト.
*/
static const RenderingData renderingData[] = {
  MakeRenderingData( 3 * 4, 0),
  MakeRenderingData( 3 * 2, 12),
  MakeRenderingData( 3 * 2, 18),
  MakeRenderingData( 3 * 2, 24),
};

/**
* Uniform Bufferのバインディング・ポイント.
*/
enum BindingPoint {
  BindingPoint_Vertex,
  BindingPoint_Light,

  countof_BindingPoint, ///< 定義したバインディング・ポイントの数.
};

/**
* ポストエフェクトパラメータ.
*/
struct PostEffectData
{
  glm::mat4x4 matColor; ///< 色変換行列.
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
  glDeleteBuffers(1, &ibo);
  glDeleteBuffers(1, &vbo);
  return vao;
}

/**
*
*/
Game::~Game()
{
  if (vao) {
    glDeleteVertexArrays(1, &vao);
  }
}

/**
*
*/
bool Game::Init()
{
  if (!isInitialized) {
    if (!InitImpl()) {
      this->~Game();
      new (this) Game;
      return false;
    }
    isInitialized = true;
  }
  return true;
}

/**
*
*/
Game::UpdateFunc Game::SetUpdateFunc(const UpdateFunc& func)
{
  UpdateFunc old = updateFunc;
  updateFunc = func;
  return old;
}

/**
*
*/
bool Game::InitImpl()
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

  tex = Texture::LoadFromFile("Res/Sample.bmp");
  texSample = Texture::LoadFromFile("Res/Model/Toroid.bmp");
  if (!tex || !texSample) {
    return false;
  }

  meshBuffer = Mesh::Buffer::Create(10 * 1024, 30 * 1024);
  if (!meshBuffer) {
    return false;
  }
  meshBuffer->LoadMeshFromFile("Res/Model/Toroid.fbx");
  sampleMesh[0] = meshBuffer->GetMesh("Toroid");
  sampleMesh[1] = meshBuffer->GetMesh("Spario");
  for (size_t i = 0; i < _countof(sampleMesh); ++i) {
    if (!sampleMesh[i]) {
      return false;
    }
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
*
*/
void Game::Update(double delta)
{
  if (updateFunc) {
    updateFunc(delta);
  }
}

/**
*
*/
void Game::Render() const
{
  //  glEnable(GL_CULL_FACE);

  glBindFramebuffer(GL_FRAMEBUFFER, offscreen->GetFramebuffer());
  glEnable(GL_DEPTH_TEST);
  glViewport(0, 0, 800, 600);
  glScissor(0, 0, 800, 600);
  glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
  glClearDepth(1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  progTutorial->UseProgram();

  {
    static float texRot = 0;
    //texRot += 0.05f;
    if (texRot >= 360) { texRot -= 360; }
    const glm::mat4x4 matProj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    const glm::mat4x4 matView = glm::lookAt(viewPos, viewTarget, viewUp);
    const glm::mat4x4 matModel = glm::scale(glm::mat4(), glm::vec3(1, 1, 1));

    TransformationData transData[11];
    transData[0].matM = matModel;
    transData[0].matMVP = matProj * matView * matModel;
    glm::mat4 matTex = glm::translate(glm::mat4(), glm::vec3(0.5f, 0.5f, 0));
    matTex = glm::rotate(matTex, glm::radians(texRot), glm::vec3(0, 0, 1));
    matTex = glm::translate(matTex, glm::vec3(-0.5f, -0.5f, 0));
    transData[0].matTex = matTex;
    uboTrans->BufferSubData(&transData);

    uboLight->BufferSubData(&lightData);

    progTutorial->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, tex->Id());

    glBindVertexArray(vao);
    uboTrans->BindBufferRange(0, sizeof(TransformationData));
    glDrawElements(GL_TRIANGLES, renderingData[0].size, GL_UNSIGNED_INT, renderingData[0].offset);

    progTutorial->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, texSample->Id());
    meshBuffer->BindVAO();
    meshBuffer->GetMesh("Toroid")->Draw(meshBuffer);

    entityBuffer->Update(1.0f / 60.0f, matView, matProj);
    entityBuffer->Draw(meshBuffer);
  }

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

  for (int i = bloomBufferCount - 1; i > 0 ; --i) {
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
