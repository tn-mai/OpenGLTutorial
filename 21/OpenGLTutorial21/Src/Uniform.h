/**
* @file Uniform.h
*/
#ifndef UNIFORM_H_INCLUDED
#define UNIFORM_H_INCLUDED
#include <glm/glm.hpp>

namespace Uniform {

static const int maxViewCount = 4;

/**
* 座標変換データ.
*/
struct VertexData
{
  glm::mat4 matMVP[maxViewCount];
  glm::mat4 matDepthMVP;
  glm::mat4 matModel;
  glm::mat3x4 matNormal;
  glm::vec4 color;
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

static const int maxLightCount = 4; ///< ライトの数.

/**
* ライティングパラメータ.
*/
struct LightingData
{
  glm::vec4 eyePos[maxViewCount]; ///< 視点.
  glm::vec4 ambientColor; ///< 環境光.
  PointLight light[maxLightCount]; ///< ライトのリスト.
};

/**
* ポストエフェクトパラメータ.
*/
struct PostEffectData
{
  glm::mat4x4 matColor; ///< 色変換行列.
  float lumScale;
  float bloomThreshould;
};

} // namespace Uniform

#endif // UNIFORM_H_INCLUDED