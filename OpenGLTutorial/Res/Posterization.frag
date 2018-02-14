#version 410

layout(location=0) in vec2 inTexCoord;

out vec4 fragColor;

uniform sampler2D colorSampler;

void main()
{
  const float numShades = 3; // âeÇÃíiêî.
  fragColor = texture(colorSampler, inTexCoord);
  fragColor.rgb = ceil(fragColor.rgb * numShades) * (1 / numShades);
}
