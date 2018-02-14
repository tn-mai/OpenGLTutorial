#version 410

// in�ϐ�.
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexCoord;
layout(location=2) in vec3 inWorldPosition;
layout(location=3) in vec3 inWorldNormal;

// out�ϐ�.
out vec4 fragColor;

// uniform�ϐ�.
uniform sampler2D colorSampler;

const int maxLightCount = 4; // ���C�g�̐�.

// ���C�g�f�[�^(�_����).
struct PointLight
{
  vec4 position; //���W(���[���h���W�n).
  vec4 color; // ���邳.
};

// ���C�e�B���O�p�����[�^.
layout(std140) uniform LightData
{
  vec4 ambientColor; // ����.
  PointLight light[maxLightCount]; // ���C�g�̃��X�g.
} lightData;

void main()
{
  fragColor = inColor * texture(colorSampler, inTexCoord);
  vec3 lightColor = lightData.ambientColor.rgb;
  for (int i = 0; i < maxLightCount; ++i) {
    vec3 lightVec = lightData.light[i].position.xyz - inWorldPosition;
    float lightPower = 1 / dot(lightVec, lightVec);
    float cosTheta = clamp(dot(inWorldNormal, normalize(lightVec)), 0, 1);
    lightColor += lightData.light[i].color.rgb * cosTheta * lightPower;
  }
  fragColor.rgb *= lightColor;
}