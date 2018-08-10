#version 410

layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexCoord;

out vec4 fragColor;

uniform sampler2D colorSampler;

void main()
{
  fragColor = inColor * texture(colorSampler, inTexCoord);
}