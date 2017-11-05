#version 410

layout(location=0) in vec4 inColor;
layout(location=1) in vec2 inTexCoord;

out vec4 fragColor;

uniform sampler2D colorSampler;

// �|�X�g�G�t�F�N�g�f�[�^.
layout(std140) uniform PostEffectData {
	mat4x4 matColor; // �F�ϊ��s��.
	float lumScale;
	float bloomThreshould;
} postEffect;

/**
* �P�x���v�Z����.
*
* @param rgb RGB�J���[.
*
* @return rgb����ITU-BT.2020�ɂ��������Čv�Z�����P�x.
*/
float luminance(vec3 rgb)
{
	return log(dot(rgb, vec3(0.2627, 0.678, 0.0593)) + 0.0001);
}

void main() {
	vec3 threshould = vec3(postEffect.bloomThreshould);
	vec4 ts;
	ts.xy = vec2(1.0) / vec2(textureSize(colorSampler, 0));
	ts.zw = -ts.xy;

	vec3 rgb = max(texture(colorSampler, inTexCoord + ts.xy).rgb, 0);
	fragColor.a = luminance(rgb);
	fragColor.rgb = max(rgb, threshould) - threshould;

	rgb = max(texture(colorSampler, inTexCoord + ts.zy).rgb, 0);
	fragColor.a += luminance(rgb);
	fragColor.rgb += max(rgb, threshould) - threshould;

	rgb = max(texture(colorSampler, inTexCoord + ts.xw).rgb, 0);
	fragColor.a += luminance(rgb);
	fragColor.rgb += max(rgb, threshould) - threshould;

	rgb = max(texture(colorSampler, inTexCoord + ts.zw).rgb, 0);
	fragColor.a += luminance(rgb);
	fragColor.rgb += max(rgb, threshould) - threshould;

	fragColor *= 1.0 / 4.0;
	fragColor *= inColor;
}