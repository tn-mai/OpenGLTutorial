/**
* @file Uniform.h
*/
#ifndef UNIFORM_H_INCLUDED
#define UNIFORM_H_INCLUDED
#include <glm/glm.hpp>

namespace Uniform {

static const int maxViewCount = 4;

/**
* ���W�ϊ��f�[�^.
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
* ���C�g�f�[�^(�_����).
*/
struct PointLight
{
  glm::vec4 position; ///< ���W(���[���h���W�n).
  glm::vec4 color; ///< ���邳.
};

static const int maxLightCount = 4; ///< ���C�g�̐�.

/**
* ���C�e�B���O�p�����[�^.
*/
struct LightingData
{
  glm::vec4 eyePos[maxViewCount]; ///< ���_.
  glm::vec4 ambientColor; ///< ����.
  PointLight light[maxLightCount]; ///< ���C�g�̃��X�g.
};

/**
* �|�X�g�G�t�F�N�g�p�����[�^.
*/
struct PostEffectData
{
  glm::mat4x4 matColor; ///< �F�ϊ��s��.
  float lumScale;
  float bloomThreshould;
};

} // namespace Uniform

#endif // UNIFORM_H_INCLUDED