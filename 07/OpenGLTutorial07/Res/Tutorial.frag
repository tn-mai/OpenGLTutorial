#version 410

layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexCoord;
layout(location=2) in vec3 inWorldPosition;
layout(location=3) in vec3 inNormal;

out vec4 fragColor;

const int maxLight = 4;

struct PointLight
{
  vec4 position;
  vec4 color;
};

layout(std140) uniform LightingData
{
  vec4 ambientColor;
  PointLight light[maxLight];
} lightingData;

uniform sampler2D colorSampler;

void main() {
  vec3 lightColor = lightingData.ambientColor.rgb;
  for (int i = 0; i < maxLight; ++i) {
    vec3 lightVector = lightingData.light[i].position.xyz - inWorldPosition;
    float lightPower = 1.0 / dot(lightVector, lightVector);
    float cosTheta = clamp(dot(vec3(0, 0, 1), normalize(lightVector)), 0, 1);
    lightColor += lightingData.light[i].color.rgb * cosTheta * lightPower;
  }
  fragColor = inColor * texture(colorSampler, inTexCoord);
#if 1
  fragColor.rgb *= lightColor;
#else
  const float numShades = 2;
  fragColor.rgb *= ceil(lightColor * numShades) * (1.0 / numShades);
#endif
}
