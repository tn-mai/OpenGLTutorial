#version 410

layout(location=0) in vec3 vPosition;
layout(location=1) in vec4 vColor;
layout(location=2) in vec2 vTexCoord;

layout(location=0) out vec4 outColor;
layout(location=1) out vec2 outTexCoord;
layout(location=2) out vec3 outPosition;
layout(location=3) out vec3 outNormal;

struct PointLight
{
	vec4 position;
	vec4 color;
};

/**
* 頂点シェーダ入力.
*/
layout(std140) uniform TransformationData
{
	mat4 matMV;
	mat4 matMVP;
	mat4 matTex;
	vec4 lightPosition;
	vec4 lightColor;
	vec4 ambientColor;
} transformationData;

void main() {
  vec3 lightVec = transformationData.lightPosition.xyz - vPosition;
  float lightPower = 1 / dot(lightVec, lightVec);
  float cosTheta = clamp(dot(vec3(0, 0, 1), normalize(lightVec)), 0, 1);
  outColor = vColor;
  outColor.rgb *= cosTheta * lightPower * transformationData.lightColor.rgb + transformationData.ambientColor.rgb;

  outNormal = mat3(transformationData.matMV) * vec3(0, 0, 1);
  outTexCoord = (transformationData.matTex * vec4(vTexCoord, 0, 1)).xy;
  outPosition = (transformationData.matMV * vec4(vPosition, 1.0)).xyz;
  gl_Position = transformationData.matMVP * vec4(vPosition, 1.0);
}