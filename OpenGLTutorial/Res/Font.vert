#version 410

layout(location=0) in vec2 vPosition;
layout(location=1) in vec2 vTexCoord;
layout(location=2) in vec4 vColor;

layout(location=0) out vec2 outTexCoord;
layout(location=1) out vec4 outColor;

void main()
{
  outTexCoord = vTexCoord;
  outColor = vColor;
  gl_Position = vec4(vPosition, 0, 1);
}