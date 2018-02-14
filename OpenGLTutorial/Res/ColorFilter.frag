#version 410

layout(location=0) in vec2 inTexCoord;

out vec4 fragColor;

uniform sampler2D colorSampler;

// ポストエフェクトデータ.
layout(std140) uniform PostEffectData
{
  mat4x4 matColor; // 色変換行列.
} postEffect;

void main()
{
  fragColor = texture(colorSampler, inTexCoord);
  fragColor.rgb = (postEffect.matColor * vec4(fragColor.rgb, 1)).rgb;
}