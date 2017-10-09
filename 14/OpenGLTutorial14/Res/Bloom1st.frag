#version 410

layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexCoord;

out vec4 fragColor;

uniform sampler2D colorSampler;

void main() {
	vec3 threshould = vec3(1.0);
	vec4 ts;
	ts.xy = vec2(1.0) / vec2(textureSize(colorSampler, 0));
	ts.zw = -ts.xy;
	fragColor.rgb = max(texture(colorSampler, inTexCoord + ts.xy).rgb, threshould) - threshould;
	fragColor.rgb += max(texture(colorSampler, inTexCoord + ts.zy).rgb, threshould) - threshould;
	fragColor.rgb += max(texture(colorSampler, inTexCoord + ts.xw).rgb, threshould) - threshould;
	fragColor.rgb += max(texture(colorSampler, inTexCoord + ts.zw).rgb, threshould) - threshould;
	fragColor.rgb *= 1.0 / 4.0;
	fragColor.a = 1.0;
	fragColor *= inColor;
}