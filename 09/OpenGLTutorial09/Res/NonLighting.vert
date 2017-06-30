#version 410

layout(location=0) in vec3 vPosition;
layout(location=1) in vec4 vColor;
layout(location=2) in vec2 vTexCoord;

layout(location=0) out vec4 outColor;
layout(location=1) out vec2 outTexCoord;

/**
* ���_�V�F�[�_����.
*/
layout(std140) uniform TransformationData
{
	mat4 matMVP;
	mat4 matModel;
	mat3x4 matNormal;
	vec4 color;
	mat4 matTex;
} transformationData;

void main() {
  outColor = vColor * transformationData.color;
  outTexCoord = (transformationData.matTex * vec4(vTexCoord, 0, 1)).xy;
  gl_Position = transformationData.matMVP * vec4(vPosition, 1.0);
}