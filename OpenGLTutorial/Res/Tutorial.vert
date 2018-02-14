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
  mat4 matModel;
  mat3x4 matNormal;
  vec4 color;
} vertexData;

void main()
{
  outColor = vertexData.color * vColor;
  outTexCoord = vTexCoord;
  outWorldPosition = (vertexData.matModel * vec4(vPosition, 1.0)).xyz;
  outWorldNormal = mat3(vertexData.matNormal) * vNormal;
  gl_Position = vertexData.matMVP * vec4(vPosition, 1.0);
}
