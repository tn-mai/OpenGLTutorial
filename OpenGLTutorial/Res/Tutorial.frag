#version 410

// in�ϐ�.
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexCoord;

// out�ϐ�.
out vec4 fragColor;

// uniform�ϐ�.
uniform sampler2D colorSampler;

void main()
{
  fragColor = inColor * texture(colorSampler, inTexCoord);
}