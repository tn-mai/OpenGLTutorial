#version 410

layout(location=0) in vec2 vPosition;
layout(location=1) in vec2 vTexCoord;
layout(location=2) in vec4 vColor;
layout(location=3) in vec4 vSubColor;
layout(location=4) in vec2 vThicknessAndOutline;

layout(location=0) out vec2 outTexCoord;
layout(location=1) out vec4 outColor;
layout(location=2) out vec4 outSubColor;
layout(location=3) out vec2 outThicknessAndOutline;

void main() {
  outTexCoord = vTexCoord;
  outColor = vColor;
  outSubColor = vSubColor;
#if 1
  outThicknessAndOutline.x = vThicknessAndOutline.x;
  outThicknessAndOutline.y = 0.5 - vThicknessAndOutline.y * 0.5;
#else
  outThicknessAndOutline = vThicknessAndOutline;
#endif
  gl_Position = vec4(vPosition, 0, 1);
}
