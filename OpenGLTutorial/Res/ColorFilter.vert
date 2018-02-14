#version 410

layout(location=0) in vec3 vPosition;
layout(location=2) in vec2 vTexCoord;

layout(location=0) out vec2 outTexCoord;

void main()
{
  outTexCoord = vTexCoord;
  gl_Position = vec4(vPosition, 1);
}
