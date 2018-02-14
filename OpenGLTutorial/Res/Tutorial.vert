#version 410

// in変数.
layout(location=0) in vec3 vPosition;
layout(location=1) in vec4 vColor;
layout(location=2) in vec2 vTexCoord;
layout(location=3) in vec3 vNormal;

// out変数.
layout(location=0) out vec4 outColor;
layout(location=1) out vec2 outTexCoord;
layout(location=2) out vec3 outWorldPosition;
layout(location=3) out vec3 outWorldNormal;

/**
* 頂点シェーダのパラメータ.
*/
layout(std140) uniform VertexData
{
  mat4 matMVP;
} vertexData;

void main()
{
  outColor = vColor;
  outTexCoord = vTexCoord;
  outWorldPosition = vPosition;
  outWorldNormal = vNormal;
  gl_Position = vertexData.matMVP * vec4(vPosition, 1.0);
}
