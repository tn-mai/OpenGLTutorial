#version 410

layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexCoord;

out vec4 fragColor;

uniform sampler2D colorSampler[3];

// ポストエフェクトデータ.
layout(std140) uniform PostEffectData {
	mat4x4 matColor; // 色変換行列.
} postEffect;

void main() {
	vec4 ts;
	ts.xy = vec2(0.25) / vec2(textureSize(colorSampler[1], 0));
	ts.zw = -ts.xy;
	vec3 bloom = texture(colorSampler[1], inTexCoord + ts.xy).rgb;
	bloom += texture(colorSampler[1], inTexCoord + ts.zy).rgb;
	bloom += texture(colorSampler[1], inTexCoord + ts.xw).rgb;
	bloom += texture(colorSampler[1], inTexCoord + ts.zw).rgb;

	fragColor.rgb = texture(colorSampler[0], inTexCoord).rgb;
	fragColor.rgb += bloom * (2.0 / 4.0);
	fragColor.rgb += texture(colorSampler[2], inTexCoord).rgb * 0.5;

	//fragColor.rgb = color;
	fragColor.a = 1.0;
	fragColor *= inColor;
	const float numShades = 3;
	//fragColor.rgb = ceil(color * numShades) * (1.0 / numShades);
	//fragColor.rgb = color * ceil(dot(vec3(0.299, 0.587, 0.114) * numShades, color)) * (1.0 / numShades);
	fragColor.rgb = (postEffect.matColor * vec4(fragColor.rgb, 1)).rgb;
}