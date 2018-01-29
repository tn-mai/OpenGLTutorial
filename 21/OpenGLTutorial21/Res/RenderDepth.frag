#version 410

layout(location=1) in vec2 inTexCoord;

layout(location = 0) out float fragDepth;

uniform sampler2D colorSampler;

void main()
{
  float a = texture(colorSampler, inTexCoord).a;
  if (a <= 0.1) {
    discard;
  }
  fragDepth = gl_FragCoord.z;
}