#version 410

layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexCoord;
layout(location=2) in vec3 inWorldPosition;
layout(location=3) in mat3 inTBN;

out vec4 fragColor;

const int maxLight = 4;

struct PointLight
{
  vec4 position;
  vec4 color;
};

layout(std140) uniform LightingData
{
  vec4 eyePos;
  vec4 ambientColor;
  PointLight light[maxLight];
} lightingData;

uniform sampler2D colorSampler[2];

const float shininess = 2;
const float normFactor = (shininess + 2) * (1.0 / (2.0 * 3.1415926)) * 0.0001;

void main()
{
  vec3 normal = texture(colorSampler[1], inTexCoord).xyz;
  normal.z = sqrt(1.0 - dot(normal.xy, normal.xy));
  normal = inTBN * normal;
  vec3 lightColor = lightingData.ambientColor.rgb;
  vec3 specularColor = vec3(0);
  for (int i = 0; i < maxLight; ++i) {
    vec3 lightVector = lightingData.light[i].position.xyz - inWorldPosition;
    float lightPower = 1.0 / dot(lightVector, lightVector);
	vec3 normalizedLightVector = normalize(lightVector);
    float cosTheta = clamp(dot(normal, normalizedLightVector), 0, 1);
    lightColor += lightingData.light[i].color.rgb * cosTheta * lightPower;

    vec3 eyeVector = normalize(lightingData.eyePos.xyz - lightingData.light[i].position.xyz);
    specularColor += lightingData.light[i].color.rgb * pow(max(dot(eyeVector, reflect(normalizedLightVector, normal)), 0), shininess);
  }
  fragColor = inColor * texture(colorSampler[0], inTexCoord);
  fragColor.rgb *= lightColor;
  fragColor.rgb += specularColor * normFactor;
}
