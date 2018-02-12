#version 410

// in変数.
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexCoord;

// out変数.
out vec4 fragColor;

// uniform変数.
uniform sampler2D colorSampler;

void main()
{
  fragColor = inColor * texture(colorSampler, inTexCoord);
}