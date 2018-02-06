#version 410

layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexCoord;
layout(location=2) in vec3 inWorldPosition;
layout(location=3) in mat3 inTBN;
layout(location=6) in vec3 inDepthCoord;

out vec4 fragColor;

const int maxLight = 4;

struct PointLight
{
  vec4 position;
  vec4 color;
};

layout(std140) uniform LightingData
{
  vec4 eyePos[4];
  vec4 ambientColor;
  PointLight light[maxLight];
} lightingData;

uniform int viewIndex;
uniform sampler2D colorSampler[2];
uniform sampler2DShadow depthSampler;

const float shininess = 2;
const float normFactor = (shininess + 2) * (1.0 / (2.0 * 3.1415926));

const float softShadowScale = 1.0 / 1600.0;
const vec3 poissonDisk[4] = vec3[](
  vec3( -0.94201624, -0.39906216, 0 ) * softShadowScale,
  vec3( 0.94558609, -0.76890725, 0 ) * softShadowScale,
  vec3( -0.094184101, -0.92938870, 0 ) * softShadowScale,
  vec3( 0.34495938, 0.29387760, 0 ) * softShadowScale
);

float ShadowRatio(float bias)
{
  vec3 coord = inDepthCoord;
  coord.z -= bias;
#if 1
  float visibility = 0.0;
  for (int i = 0; i < 4; ++i) {
    visibility += texture(depthSampler, coord + poissonDisk[i]);
  }
  return visibility * (1.0 / 4.0) * 0.5 + 0.5;
#else
  return texture(depthSampler, coord) * 0.5 + 0.5;
#endif
}

void main()
{
  vec3 normal = texture(colorSampler[1], inTexCoord).xyz * 2 - 1;
  //normal.z = sqrt(1.0 - dot(normal.xy, normal.xy));
  normal = inTBN * normal;
  vec3 lightColor = vec3(0);
  vec3 specularColor = vec3(0);
  for (int i = 0; i < maxLight; ++i) {
    vec3 lightVector = lightingData.light[i].position.xyz - inWorldPosition;
    float lightPower = 1.0 / (dot(lightVector, lightVector) + 0.00001);
	vec3 normalizedLightVector = normalize(lightVector);
    float cosTheta = clamp(dot(normal, normalizedLightVector), 0, 1);
    lightColor += lightingData.light[i].color.rgb * cosTheta * lightPower;

    vec3 eyeVector = normalize(lightingData.eyePos[viewIndex].xyz - lightingData.light[i].position.xyz);
    specularColor += lightingData.light[i].color.rgb * pow(max(dot(eyeVector, reflect(normalizedLightVector, normal)), 0), shininess) * lightPower * 0.25;
  }
  fragColor = inColor * texture(colorSampler[0], inTexCoord);

  float cosTheta = clamp(dot(normal, normalize(lightingData.light[0].position.xyz - inWorldPosition)), 0, 1);
  float depthBias = 0.005 * tan(acos(cosTheta));
  depthBias = clamp(depthBias, 0, 0.01);
  float shadow = ShadowRatio(depthBias);
  fragColor.rgb *= lightColor * shadow + lightingData.ambientColor.rgb;
  fragColor.rgb += specularColor * normFactor * shadow;
}
