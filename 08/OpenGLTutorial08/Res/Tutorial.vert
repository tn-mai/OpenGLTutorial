#version 410

layout(location=0) in vec3 vPosition;
layout(location=1) in vec4 vColor;
layout(location=2) in vec2 vTexCoord;
layout(location=3) in vec3 vNormal;

layout(location=0) out vec4 outColor;
layout(location=1) out vec2 outTexCoord;
layout(location=2) out vec3 outWorldPosition;
layout(location=3) out vec3 outNormal;

/**
* 頂点シェーダ入力.
*/
layout(std140) uniform TransformationData
{
	mat4 matM;
	mat4 matMVP;
	mat4 matTex;
} transformationData;

void main() {
  outColor = vColor;
  outTexCoord = (transformationData.matTex * vec4(vTexCoord, 0, 1)).xy;
  outWorldPosition = (transformationData.matM * vec4(vPosition, 1.0)).xyz;
  outNormal = mat3(transformationData.matM) * vNormal;
  gl_Position = transformationData.matMVP * vec4(vPosition, 1.0);
}