#version 410
layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexCoord;
uniform sampler2D colorSampler;
out vec4 fragColor;
void main() {
  fragColor = inColor * texture(colorSampler, inTexCoord);
}
