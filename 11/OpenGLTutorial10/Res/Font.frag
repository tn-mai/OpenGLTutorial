#version 410

layout(location=0) in vec2 inTexCoord;
layout(location=1) in vec4 inColor;
layout(location=2) in vec4 inSubColor;
layout(location=3) in vec2 inThicknessAndOutline;

out vec4 fragColor;

uniform sampler2D colorSampler;

const float smoothing = 1.0 / 16.0;

void main()
{
  float distance = 1.0 - texture(colorSampler, inTexCoord).r;
#if 1
  float outline = smoothstep(inThicknessAndOutline.x - smoothing, inThicknessAndOutline.x + smoothing, distance);
  fragColor = mix(inSubColor, inColor, outline);
  fragColor.a *= smoothstep(inThicknessAndOutline.y - smoothing, inThicknessAndOutline.y + smoothing, distance);
#elif 0
  fragColor = inColor;
  fragColor.a *= texture(colorSampler, inTexCoord).r;
#else
  float alpha = smoothstep(inThicknessAndOutline.x - smoothing, inThicknessAndOutline.x + smoothing, distance);
  vec4 text = vec4(inColor.rgb, inColor.a * alpha);
  float shadowDist = 1.0 - texture(colorSampler, inTexCoord + vec2(-inThicknessAndOutline.y * 4.0 / 512.0, inThicknessAndOutline.y * 4.0 / 512.0)).r;
  float shadowAlpha = smoothstep(inThicknessAndOutline.x - smoothing, inThicknessAndOutline.x + smoothing, shadowDist);
  vec4 shadow = vec4(inSubColor.rgb, inSubColor.a * shadowAlpha);
  fragColor = mix(shadow, text, alpha);
#endif
}
