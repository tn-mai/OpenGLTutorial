#version 410

// in�ϐ�.
layout(location=0) in vec3 vPosition;
layout(location=1) in vec4 vColor;
layout(location=2) in vec2 vTexCoord;

// out�ϐ�.
layout(location=0) out vec4 outColor;
layout(location=1) out vec2 outTexCoord;
layout(location=2) out vec3 outWorldPosition;

/**
* ���_�V�F�[�_�̃p�����[�^.
*/
layout(std140) uniform VertexData
{
  mat4 matMVP;
} vertexData;

void main()
{
  outColor = vColor;
  outTexCoord = vTexCoord;
  outWorldPosition = vPosition;
  gl_Position = vertexData.matMVP * vec4(vPosition, 1.0);
}
