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

/**
* ゲームエンジンのインスタンスを取得する.
*
* @return ゲームエンジンのインスタンス.
*/
GameEngine& GameEngine::Instance() {
  static GameEngine instance;
  return instance;
}

/**
* デストラクタ.
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
* 状態更新関数を設定する.
*
* @param func 設定する更新関数.
*/
void GameEngine::UpdateFunc(const UpdateFuncType& func)
{
  updateFunc = func;
}

/**
* 状態更新関数を取得する.
*
* @return 設定されている更新関数.
*/
const GameEngine::UpdateFuncType& GameEngine::UpdateFunc() const
{
  return updateFunc;
}

/**
* ゲームエンジンを初期化する.
*
* @param w     ウィンドウの描画範囲の幅(ピクセル).
* @param h     ウィンドウの描画範囲の高さ(ピクセル).
* @param title ウィンドウタイトル(UTF-8の0終端文字列).
*
* @retval true  初期化成功.
* @retval false 初期化失敗.
*
* Update, Render関数などを呼び出す前に、一度だけ呼び出しておく必要がある.
* 一度初期化に成功すると、以後の呼び出しではなにもせずにtrueを返す.
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
* ゲームの状態を更新する.
*
* @param delta 前回の更新からの経過時間(秒).
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
* 優先順でソートされたカメラインデックス配列を作成する.
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
* 影を描画する.
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
* ゲームの状態を描画する.
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
* ゲームを実行する.
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
* ゲームパッドの状態を取得する.
*/
const GamePad& GameEngine::GetGamePad(int id) const
{
  return GLFWEW::Window::Instance().GetGamePad(id);
}

/**
* テクスチャを読み込む.
*
* @param filename テクスチャファイル名.
*
* @retval true  読み込み成功.
* @retval false 読み込み失敗.
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
* テクスチャを取得する.
*
* @param filename テクスチャファイル名.
*
*
* @return filenameに対応するテクスチャオブジェクト.
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
* メッシュを読み込む.
*
* @param filename メッシュファイル名.
*
* @retval true  読み込み成功.
* @retval false 読み込み失敗.
*/
bool GameEngine::LoadMeshFromFile(const char* filename)
{
  return meshBuffer->LoadMeshFromFile(filename);
}

/**
* メッシュを取得する.
*
* @param name メッシュ名.
*
* @return nameに対応するメッシュオブジェクト.
*/
const Mesh::MeshPtr& GameEngine::GetMesh(const char* name)
{
  return meshBuffer->GetMesh(name);
}

/**
* エンティティを追加する.
*
* @param groupId  エンティティのグループID.
* @param position エンティティの座標.
* @param meshName エンティティの表示に使用するメッシュ名.
* @param texName  エンティティの表示に使うテクスチャファイル名.
* @param func     エンティティの状態を更新する関数(または関数オブジェクト).
* @param shader   エンティティの表示に使うシェーダ名.
*
* @return 追加したエンティティへのポインタ.
*         これ以上エンティティを追加できない場合はnullptrが返される.
*         回転や拡大率はこのポインタ経由で設定する.
*         なお、このポインタをアプリケーション側で保持する必要はない.
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
*　エンティティを削除する.
*
* @param 削除するエンティティのポインタ.
*/
void GameEngine::RemoveEntity(Entity::Entity* e)
{
  entityBuffer->RemoveEntity(e);
}

/**
* 全てのエンティティを削除する.
*/
void GameEngine::RemoveAllEntity()
{
  entityBuffer->RemoveAllEntity();
}

/**
* ライトを設定する.
*
* @param indes  設定するライトのインデックス.
* @param light  ライトパラメータ.
*/
void GameEngine::Light(int index, const Uniform::PointLight& light)
{
  if (index < 0 || index >= Uniform::maxLightCount) {
    std::cerr << "WARNING: '" << index << "'は不正なライトインデックスです" << std::endl;
    return;
  }
  lightData.light[index] = light;
}

/**
* ライトを取得する.
*
* @param index 取得するライトのインデックス.
*
* @return ライトパラメータ.
*/
const Uniform::PointLight& GameEngine::Light(int index) const
{
  if (index < 0 || index >= Uniform::maxLightCount) {
    std::cerr << "WARNING: '" << index << "'は不正なライトインデックスです" << std::endl;
    static const Uniform::PointLight dummy;
    return dummy;
  }
  return lightData.light[index];
}

/**
* 環境光を設定する.
*
* @param color 環境光の明るさ.
*/
void GameEngine::AmbientLight(const glm::vec4& color)
{
  lightData.ambientColor = color;
}

/**
* 環境光を取得する.
*
* @return 環境光の明るさ.
*/
const glm::vec4& GameEngine::AmbientLight() const
{
  return lightData.ambientColor;
}

/**
* 視点の位置と姿勢を設定する.
*
* @param index カメラのインデックス.
* @param cam   設定するカメラデータ.
*
* 指定されたインデックスのカメラをアクティブ化し、状態を設定する.
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
* 視点の位置と姿勢を取得する.
*
* @param index カメラのインデックス.
* @return カメラデータ.
*/
const GameEngine::CameraData& GameEngine::Camera(size_t index) const
{
  return camera[index].camera;
}

/**
* 全てのカメラの状態を初期状態に戻す.
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
* 乱数オブジェクトを取得する.
*
* @return 乱数オブジェクト.
*/
std::mt19937& GameEngine::Rand()
{
  return rand;
}

/**
* 衝突解決ハンドラを設定する.
*
* @param gid0    衝突対象のグループID.
* @param gid1    衝突対象のグループID.
* @param handler 衝突解決ハンドラ.
*
* 衝突が発生し衝突解決ハンドラが呼びされるとき、より小さいグループIDを持つエンティティから先に渡される.
* ここで指定したグループIDの順序とは無関係であることに注意すること.
* ex)
*   CollisionHandler(10, 1, Func)
*   というコードでハンドラを登録したとする. 衝突が発生すると、
*   Func(グループID=1のエンティティ、グループID=10のエンティティ)
*   のように呼び出される.
*/
void GameEngine::CollisionHandler(int gid0, int gid1, Entity::CollisionHandlerType handler)
{
  entityBuffer->CollisionHandler(gid0, gid1, handler);
}

/**
* 衝突解決ハンドラを取得する.
*
* @param gid0 衝突対象のグループID.
* @param gid1 衝突対象のグループID.
*
* @return 衝突解決ハンドラ.
*/
const Entity::CollisionHandlerType& GameEngine::CollisionHandler(int gid0, int gid1) const
{
  return entityBuffer->CollisionHandler(gid0, gid1);
}

/**
* 衝突解決ハンドラのリストをクリアする.
*/
void GameEngine::ClearCollisionHandlerList()
{
  entityBuffer->ClearCollisionHandlerList();
}

/**
* オーディオを初期化する.
*/
bool GameEngine::InitAudio(const char* acfPath, const char* acbPath, const char* awbPath, const char* dspBusName)
{
  return Audio::Initialize(acfPath, acbPath, awbPath, dspBusName);
}

/**
* 音を再生する.
*/
void GameEngine::PlayAudio(int playerId, int cueId)
{
  Audio::Play(playerId, cueId);
}

/**
* 音を停止する.
*/
void GameEngine::StopAudio(int playerId)
{
  Audio::Stop(playerId);
}

/**
* リソーススタックに新しいリソースレベルを作成する.
*/
void GameEngine::PushLevel()
{
  meshBuffer->PushLevel();
  textureMapStack.push_back(TextureMap());
}

/**
* リソーススタックの末尾のリソースレベルを除去する.
*/
void GameEngine::PopLevel()
{
  meshBuffer->PopLevel();
  if (textureMapStack.size() > minimalStackSize) {
    textureMapStack.pop_back();
  }
}

/**
* 末尾のリソースレベルを空の状態にする.
*/
void GameEngine::ClearLevel()
{
  meshBuffer->ClearLevel();
  textureMapStack.back().clear();
}