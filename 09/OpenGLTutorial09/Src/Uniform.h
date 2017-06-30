/**
* @file Uniform.h
*/
#ifndef UNIFORM_H_INCLUDED
#define UNIFORM_H_INCLUDED
#include <glm/glm.hpp>

namespace Uniform {

/**
* ���W�ϊ��f�[�^.
*/
struct TransformationData
{
  glm::mat4 matMVP;
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

static const int maxLightCount = 8; ///< ���C�g�̐�.

/**
* ���C�e�B���O�p�����[�^.
*/
struct LightingData
{
  glm::vec4 ambientColor; ///< ����.
  PointLight light[maxLightCount]; ///< ���C�g�̃��X�g.
};

} // namespace Uniform

#endif // UNIFORM_H_INCLUDED