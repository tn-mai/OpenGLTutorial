/**
* @file Font.h
*/
#ifndef FONT_H_INCLUDED
#define FONT_H_INCLUDED
#include <GL/glew.h>
#include "BufferObject.h"
#include "Shader.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>

namespace Font {

struct Vertex;

/// �t�H���g���.
struct FontInfo {
  int id = -1;        ///< �����R�[�h.
  glm::u16vec2 uv[2]; ///< �t�H���g�摜�̃e�N�X�`�����W.
  glm::vec2 size;     ///< �t�H���g�摜�̕\���T�C�Y.
  glm::vec2 offset;   ///< �\���ʒu�����炷����.
  float xadvance = 0; ///< �J�[�\����i�߂鋗��.
};

/**
* �r�b�g�}�b�v�t�H���g�`��N���X.
*/
class Renderer
{
public:
  Renderer() = default;
  ~Renderer() = default;
  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer&) = delete;

  bool Init(size_t maxChar, const glm::vec2& ss);
  bool LoadFromFile(const char* filename);

  void Scale(const glm::vec2& s) { scale = s; }
  const glm::vec2& Scale() const { return scale; }
  void Color(const glm::vec4& c);
  glm::vec4 Color() const;

  void MapBuffer();
  bool AddString(const glm::vec2& position, const char* str);
  void UnmapBuffer();
  void Draw() const;

private:
  BufferObject vbo;
  BufferObject ibo;
  VertexArrayObject vao;
  GLsizei vboCapacity = 0;        ///< VBO�Ɋi�[�\�ȍő咸�_��.
  std::vector<FontInfo> fontList; ///< �t�H���g�ʒu���̃��X�g.
  std::string texFilename;        ///< �t�H���g�e�N�X�`���t�@�C����.
  Shader::ProgramPtr progFont;    ///< �t�H���g�`��p�V�F�[�_�v���O����.
  glm::vec2 reciprocalScreenSize; ///< ��ʃT�C�Y�̋t��.

  glm::vec2 scale = glm::vec2(1, 1); ///< �t�H���g��`�悷��Ƃ��̊g�嗦.
  glm::u8vec4 color = glm::u8vec4(255, 255, 255, 255); ///< �t�H���g��`�悷��Ƃ��̐F.
  GLsizei vboSize = 0;            ///< VBO�Ɋi�[����Ă��钸�_��.
  Vertex* pVBO = nullptr;         ///< VBO�ւ̃|�C���^.
};

} // namespace Font

#endif // FONT_H_INCLUDED
