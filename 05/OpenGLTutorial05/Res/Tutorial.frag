#version 410

layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexCoord;
layout(location=2) in vec3 inPosition;
layout(location=3) in vec3 inNormal;

out vec4 fragColor;

struct PointLight
{
  vec4 position;
  vec4 color;
};

layout(std140) uniform LightingData
{
  vec4 ambientColor;
  PointLight light[4];
} lightingData;

uniform sampler2D colorSampler;

void main() {
  vec3 lightColor = lightingData.ambientColor.rgb;
  for (int i = 0; i < 4; ++i) {
    if (lightingData.light[i].color.a != 0) {
      vec3 lightVector = (inPosition - (lightingData.light[i].position).xyz);
      float lightPower = 1.0 / dot(lightVector, lightVector);
      float cosTheta = clamp(dot(inNormal, normalize(lightVector)), 0, 1);
      lightColor += lightingData.light[i].color.rgb * cosTheta * lightPower;
    }
  }
  fragColor = inColor * texture(colorSampler, inTexCoord);
//  fragColor.rgb *= lightColor;
}
