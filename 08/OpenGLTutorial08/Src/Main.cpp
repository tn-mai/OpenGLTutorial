/**
* @file main.cpp
*/
#include "GLFWEW.h"
#include "Texture.h"
#include "Shader.h"
#include "OffscreenBuffer.h"
#include "UniformBuffer.h"
#include "Mesh.h"
#include "Entity.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <random>
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
* 座標変換データ.
*/
struct TransformationData
{
  glm::mat4 matM;
  glm::mat4 matMVP;
  glm::mat4 matTex;
};

/**
* ライトデータ(点光源).
*/
struct PointLight
{
  glm::vec4 position; ///< 座標(ワールド座標系).
  glm::vec4 color; ///< 明るさ.
};

const int maxLightCount = 8; ///< ライトの数.

/**
* ライティングパラメータ.
*/
struct LightingData
{
  glm::vec4 ambientColor; ///< 環境光.
  PointLight light[maxLightCount]; ///< ライトのリスト.
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

void UpdateToroid(Entity::Entity& entity, UniformBufferPtr& ubo, double delta, const glm::mat4& matView, const glm::mat4& matProj)
{
  float rot = glm::angle(entity.Rotation());
  rot += glm::radians(10.0f) * static_cast<float>(delta);
  if (rot > glm::pi<float>() * 2.0f) {
    rot = 0.0f;
  }
  entity.Rotation() = glm::angleAxis(rot, glm::vec3(0, 1, 0));
  TransformationData data;
  data.matM = entity.TRSMatrix();
  data.matMVP = matProj * matView * data.matM;
  entity.BufferSubData(ubo, &data);
}

void UpdateSpario(Entity::Entity& entity, UniformBufferPtr& ubo, double delta, const glm::mat4& matView, const glm::mat4& matProj)
{
  glm::vec3 pos = entity.Position();
  pos.z -= 1 * static_cast<float>(delta);
  if (pos.z < -20.0f) {
    pos.z = 20.0f;
  }
  entity.Position() = pos;
  TransformationData data;
  data.matM = entity.TRSMatrix();
  data.matMVP = matProj * matView * data.matM;
  entity.BufferSubData(ubo, &data);
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
  const UniformBufferPtr uboTrans = UniformBuffer::Create(sizeof(TransformationData), BindingPoint_Vertex, "VertexData");
  const UniformBufferPtr uboLight = UniformBuffer::Create(sizeof(LightingData), BindingPoint_Light, "LightingData");
  const UniformBufferPtr uboPostEffect = UniformBuffer::Create(sizeof(PostEffectData), 2, "PostEffectData");
  const Shader::ProgramPtr shaderProgram = Shader::Program::Create("Res/Tutorial.vert", "Res/Tutorial.frag");
  const Shader::ProgramPtr progPostEffect = Shader::Program::Create("Res/PostEffect.vert", "Res/PostEffect.frag");
  const Shader::ProgramPtr progBloom1st = Shader::Program::Create("Res/Bloom1st.vert", "Res/Bloom1st.frag");
  const Shader::ProgramPtr progComposition = Shader::Program::Create("Res/FinalComposition.vert", "Res/FinalComposition.frag");
  const Shader::ProgramPtr progSimple = Shader::Program::Create("Res/Simple.vert", "Res/Simple.frag");
  const Shader::ProgramPtr progLensFlare = Shader::Program::Create("Res/AnamorphicLensFlare.vert", "Res/AnamorphicLensFlare.frag");
  if (!vbo || !ibo || !vao || !uboTrans || !uboLight || !shaderProgram || !progPostEffect || !progBloom1st || !progComposition || !progLensFlare) {
    return 1;
  }
  shaderProgram->UniformBlockBinding("TransformationData", BindingPoint_Vertex);
  shaderProgram->UniformBlockBinding("LightingData", BindingPoint_Light);
  progComposition->UniformBlockBinding("PostEffectData", 2);

  static const uint32_t textureData[] = {
    0xffffffff, 0xffcccccc, 0xffffffff, 0xffcccccc, 0xffffffff,
    0xff888888, 0xffffffff, 0xff888888, 0xffffffff, 0xff888888,
    0xffffffff, 0xff444444, 0xffffffff, 0xff444444, 0xffffffff,
    0xff000000, 0xffffffff, 0xff000000, 0xffffffff, 0xff000000,
    0xffffffff, 0xff000000, 0xffffffff, 0xff000000, 0xffffffff,
  };
//  TexturePtr tex = Texture::Create(5, 5, GL_RGBA8, GL_RGBA, textureData);
  TexturePtr tex = Texture::LoadFromFile("Res/Sample.bmp");
  TexturePtr texSample = Texture::LoadFromFile("Res/Model/Toroid.bmp");
  if (!tex || !texSample) {
    return 1;
  }

  Mesh::BufferPtr meshBuffer = Mesh::Buffer::Create(10 * 1024, 30 * 1024);
  meshBuffer->LoadMeshFromFile("Res/Model/Toroid.fbx");
  Mesh::MeshPtr sampleMesh[2] = { meshBuffer->GetMesh("Toroid"), meshBuffer->GetMesh("Spario") };

  Entity::Buffer entityBuffer;
  entityBuffer.Initialize(1024, sizeof(TransformationData), BindingPoint_Vertex, "VertexData");
  std::mt19937 rand(time(nullptr));
  std::uniform_int_distribution<> distributerX(-10, 10);
  std::uniform_int_distribution<> distributerZ(-10, 10);
  for (int i = 0; i < 10; ++i) {
    entityBuffer.AddEntity(glm::vec3(distributerX(rand), 0, distributerZ(rand)), sampleMesh[0], texSample, shaderProgram, UpdateToroid);
  }
  for (int i = 0; i < 10; ++i) {
    entityBuffer.AddEntity(glm::vec3(distributerX(rand), 0, distributerZ(rand)), sampleMesh[1], texSample, shaderProgram, UpdateSpario);
  }
  //  glEnable(GL_CULL_FACE);

  const OffscreenBufferPtr offscreen = OffscreenBuffer::Create(800, 600,  GL_RGBA16F);
  const int bloomBufferCount = 6;
  OffscreenBufferPtr offBloom[bloomBufferCount];
  for (int i = 0, scale = 4; i < bloomBufferCount; ++i, scale *= 2) {
    const int w = (800 + scale - 1) / scale;
    const int h = (600 + scale - 1) / scale;
    offBloom[i] = OffscreenBuffer::Create(w, h, GL_RGBA8);
  }
  OffscreenBufferPtr offAnamorphic[2];
  offAnamorphic[0] = OffscreenBuffer::Create(800 / 16, 600 / 2, GL_RGBA8);
  offAnamorphic[1] = OffscreenBuffer::Create(800 / 64, 600 / 2, GL_RGBA8);

  while (!window.ShouldClose()) {
    glBindFramebuffer(GL_FRAMEBUFFER, offscreen->GetFramebuffer());
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, 800, 600);
    glScissor(0, 0, 800, 600);
    glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
    glClearDepth(1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static float degree = 0.0f;
    degree += 0.05f;
    if (degree >= 360.0f) { degree -= 360.0f; }
    const glm::vec3 viewPos = glm::rotate(glm::mat4(), glm::radians(degree), glm::vec3(0, 1, 0)) * glm::vec4(2, 3, 3, 1);

    shaderProgram->UseProgram();

    {
      static float texRot = 0;
      //texRot += 0.05f;
      if (texRot >= 360) { texRot -= 360; }
      const glm::mat4x4 matProj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
      const glm::mat4x4 matView = glm::lookAt(viewPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
      const glm::mat4x4 matModel = glm::scale(glm::mat4(), glm::vec3(1, 1, 1));

      TransformationData transData[11];
      transData[0].matM = matModel;
      transData[0].matMVP = matProj * matView * matModel;
      glm::mat4 matTex = glm::translate(glm::mat4(), glm::vec3(0.5f, 0.5f, 0));
      matTex = glm::rotate(matTex, glm::radians(texRot), glm::vec3(0, 0, 1));
      matTex = glm::translate(matTex, glm::vec3(-0.5f, -0.5f, 0));
      transData[0].matTex = matTex;
      uboTrans->BufferSubData(&transData);

      LightingData lightData;
      lightData.ambientColor = glm::vec4(0.05f, 0.1f, 0.2f, 1);
      lightData.light[0].color = glm::vec4(18, 18, 18, 1);
      lightData.light[0].position = glm::vec4(2, 2, 2, 1);
      lightData.light[1].color = glm::vec4(0.125f, 0.125f, 0.05f, 1);
      lightData.light[1].position = glm::vec4(-0.2f, 0, 0.6f, 1);
      uboLight->BufferSubData(&lightData);

      shaderProgram->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, tex->Id());

      glBindVertexArray(vao);
      uboTrans->BindBufferRange(0, sizeof(TransformationData));
      glDrawElements(GL_TRIANGLES, renderingData[0].size, GL_UNSIGNED_INT, renderingData[0].offset);

      shaderProgram->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, texSample->Id());
      meshBuffer->BindVAO();
      meshBuffer->GetMesh("Toroid")->Draw(meshBuffer);

      entityBuffer.Update(1.0f / 60.0f, matView, matProj);
      entityBuffer.Draw(meshBuffer);
     }

    glBindVertexArray(vao);

#if 0
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.5f, 0.3f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shaderProgram->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offscreen->GetTexutre());

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

    window.SwapBuffers();
  }

  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);

  return 0;
}