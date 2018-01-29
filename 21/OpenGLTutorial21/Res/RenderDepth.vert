#version 410

layout(location=0) in vec3 vPosition;
layout(location=2) in vec2 vTexCoord;

layout(location=1) out vec2 outTexCoord;

/**
* 頂点シェーダ入力.
*/
layout(std140) uniform VertexData
{
	mat4 matMVP[4];
	mat4 matDepthMVP;
	mat4 matModel;
	mat3x4 matNormal;
	vec4 color;
	mat4 matTex;
} vertexData;

void main()
{
  outTexCoord = vTexCoord;
  gl_Position = vertexData.matDepthMVP * vec4(vPosition, 1);
}