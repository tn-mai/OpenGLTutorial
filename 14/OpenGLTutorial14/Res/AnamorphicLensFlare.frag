#version 410

layout(location=0) in vec2 inTexCoord;
layout(location=1) in vec4 inColor;

out vec4 fragColor;

uniform sampler2D colorSampler;

void main() {
	float offset = 1.0 / textureSize(colorSampler, 0).x;
	vec2 coord = inTexCoord;
	vec3 result = vec3(0);
	coord.x -= 3.5 * offset;
	for (int i = 0; i < 8; ++i) {
		result += texture(colorSampler, coord).rgb;
		coord.x += offset;
	}
	result *= 1.0 / 8.0 / 2.0;
	fragColor = vec4(result, 1.0);
}