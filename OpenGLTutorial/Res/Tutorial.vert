#version 410

// in変数.
layout(location=0) in vec3 vPosition;
layout(location=1) in vec4 vColor;
layout(location=2) in vec2 vTexCoord;

// out変数.
layout(location=0) out vec4 outColor;
layout(location=1) out vec2 outTexCoord;

/**
* 頂点シェーダのパラメータ.
*/
layout(std140) uniform VertexData
{
  mat4 matMVP;
  vec4 lightPosition;
  vec4 lightColor;
  vec4 ambientColor;
} vertexData;

void main()
{
  vec3 lightVec = vertexData.lightPosition.xyz - vPosition;
  float lightPower = 1 / dot(lightVec, lightVec);
  float cosTheta = clamp(dot(vec3(0, 0, 1), normalize(lightVec)), 0, 1);
  outColor = vColor;
  outColor.rgb *= (cosTheta * lightPower * vertexData.lightColor.rgb) + vertexData.ambientColor.rgb;
  outTexCoord = vTexCoord;
  gl_Position = vertexData.matMVP * vec4(vPosition, 1.0);
}
