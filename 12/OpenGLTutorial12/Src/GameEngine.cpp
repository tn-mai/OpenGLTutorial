/**
* @file GameEngine.cpp
*/
#include "GameEngine.h"
#include "GLFWEW.h"
#include <glm/gtc/matrix_transform.hpp>
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
  progTutorial = Shader::Program::Create("Res/Tutorial.vert", "Res/Tutorial.frag");
  progPostEffect = Shader::Program::Create("Res/PostEffect.vert", "Res/PostEffect.frag");
  progBloom1st = Shader::Program::Create("Res/Bloom1st.vert", "Res/Bloom1st.frag");
  progComposition = Shader::Program::Create("Res/FinalComposition.vert", "Res/FinalComposition.frag");
  progSimple = Shader::Program::Create("Res/Simple.vert", "Res/Simple.frag");
  progLensFlare = Shader::Program::Create("Res/AnamorphicLensFlare.vert", "Res/AnamorphicLensFlare.frag");
  progNonLighting = Shader::Program::Create("Res/NonLighting.vert", "Res/NonLighting.frag");
  if (!vbo || !ibo || !vao || !uboLight || !progTutorial || !progPostEffect || !progBloom1st || !progComposition || !progLensFlare) {
    return false;
  }
  progTutorial->UniformBlockBinding("VertexData", BindingPoint_Vertex);
  progTutorial->UniformBlockBinding("LightingData", BindingPoint_Light);
  progComposition->UniformBlockBinding("PostEffectData", 2);

  meshBuffer = Mesh::Buffer::Create(10 * 1024, 30 * 1024);
  if (!meshBuffer) {
    return false;
  }

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

  fontRenderer.Init(1024, glm::vec2(800, 600));

  Audio::Initialize();

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
  const glm::mat4x4 matProj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 1.0f, 200.0f);
  const glm::mat4x4 matView = glm::lookAt(camera.position, camera.target, camera.up);
  entityBuffer->Update(delta, matView, matProj);
  fontRenderer.UnmapBuffer();
}

/**
* ゲームの状態を描画する.
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

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  uboLight->BufferSubData(&lightData);
  entityBuffer->Draw(meshBuffer);

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
  glDisable(GL_CULL_FACE);
  glDisable(GL_BLEND);

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

  Uniform::PostEffectData postEffect;
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

  fontRenderer.Draw();

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
  const auto itr = textureBuffer.find(filename);
  if (itr != textureBuffer.end()) {
    return true;
  }
  TexturePtr texture = Texture::LoadFromFile(filename, wrapMode);
  if (!texture) {
    return false;
  }
  textureBuffer.insert(std::make_pair(std::string(filename), texture));
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
  const auto itr = textureBuffer.find(filename);
  if (itr != textureBuffer.end()) {
    return itr->second;
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
*
* @return 追加したエンティティへのポインタ.
*         これ以上エンティティを追加できない場合はnullptrが返される.
*         回転や拡大率はこのポインタ経由で設定する.
*         なお、このポインタをアプリケーション側で保持する必要はない.
*/
Entity::Entity* GameEngine::AddEntity(int groupId, const glm::vec3& pos, const char* meshName, const char* texName, Entity::Entity::UpdateFuncType func, bool hasLight)
{
  const Mesh::MeshPtr& mesh = meshBuffer->GetMesh(meshName);
  const TexturePtr& tex = GetTexture(texName);
  return entityBuffer->AddEntity(groupId, pos, mesh, tex, hasLight ? progTutorial : progNonLighting, func);
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
* @param cam 設定するカメラデータ.
*/
void GameEngine::Camera(const CameraData& cam)
{
  camera = cam;
}

/**
* 視点の位置と姿勢を取得する.
*
* @return カメラデータ.
*/
const GameEngine::CameraData& GameEngine::Camera() const
{
  return camera;
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

void GameEngine::PlayAudio(int playerId, int cueId)
{
  Audio::Play(playerId, cueId);
}
