/**
* @file Font.cpp
*/
#include "Font.h"
#include "File.h"
#include "GameEngine.h"
#include <regex>
#include <iostream>
#include <cstdint>

/**
* �t�H���g�`��@�\���i�[���閼�O���.
*/
namespace Font {

/**
* �t�H���g�p���_�f�[�^�^.
*/
struct Vertex
{
  glm::vec2 position;
  glm::u16vec2 uv;
  glm::u8vec4 color;
  glm::u8vec4 subColor;
  glm::u16vec2 thicknessAndOutline;
};

/**
* �t�H���g�`��I�u�W�F�N�g������������.
*
* @param maxChar �ő�`�敶����.
*
* @retval true  ����������.
* @retval false ���������s.
*/
bool Renderer::Init(size_t maxChar)
{
  if (maxChar > USHRT_MAX / 4) {
    maxChar = USHRT_MAX / 4;
  }
  vboCapacity = static_cast<GLsizei>(4 * maxChar);
  vbo.Init(GL_ARRAY_BUFFER, sizeof(Vertex) * vboCapacity, nullptr, GL_STREAM_DRAW);
  {
    std::vector<uint16_t> tmp;
    tmp.resize(maxChar * 6);
    uint16_t* p = tmp.data();
    for (uint16_t i = 0; i < maxChar * 4; i += 4) {
      p[0] = i + 0;
      p[1] = i + 1;
      p[2] = i + 2;
      p[3] = i + 2;
      p[4] = i + 3;
      p[5] = i + 0;
      p += 6;
    }
    ibo.Init(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * 6 * maxChar, tmp.data(), GL_STATIC_DRAW);
  }
  vao.Init(vbo.Id(), ibo.Id());
  vao.Bind();
  vao.VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, position));
  vao.VertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(Vertex), offsetof(Vertex, uv));
  vao.VertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), offsetof(Vertex, color));
  vao.VertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), offsetof(Vertex, subColor));
  vao.VertexAttribPointer(4, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(Vertex), offsetof(Vertex, thicknessAndOutline));
  vao.Unbind();

  progFont = Shader::Program::Create("Res/Font.vert", "Res/Font.frag");
  if (!progFont) {
    return false;
  }
  MapBuffer();
  return true;
}

/**
* �f�X�g���N�^.
*/
Renderer::~Renderer()
{
}

/**
* �t�H���g�t�@�C����ǂݍ���.
*
* @param filename �t�H���g�t�@�C����.
*
* @retval true  �ǂݍ��ݐ���.
* @retval false �ǂݍ��ݎ��s.
*/
bool Renderer::LoadFromFile(const char* filename)
{
  std::vector<char> buffer;
  if (!File::ReadFile(filename, buffer)) {
    return false;
  }
  static const std::regex reInfo(R"#(^info face=".*" size=\d+ bold=\d+ italic=\d+ charset=".*" unicode=\d+ stretchH=\d+ smooth=\d+ aa=\d+ padding=\d+,\d+,\d+,\d+ spacing=-?\d+,-?\d+ *\r?\n)#");
  static const std::regex reCommon(R"#(^common lineHeight=\d+ base=\d+ scaleW=(\d+) scaleH=(\d+) pages=\d+ packed=\d+ *\r?\n)#");
  static const std::regex rePage(R"#(^page id=0 file="(.*)" *\r?\n)#");
  static const std::regex reChars(R"#(^chars count=(\d+) *\r?\n)#");
  static const std::regex reChar(R"#(^char id=(\d+) +x=(\d+) +y=(\d+) +width=(\d+) +height=(\d+) +xoffset=(-?\d+) +yoffset=(-?\d+) +xadvance=(-?\d+) +page=(\d+) +chnl=(\d+) *\r?\n?)#");

  const char* p = buffer.data();
  std::cmatch m;
  if (!std::regex_search(p, m, reInfo)) {
    return false;
  }
  p = m[0].second;

  if (!std::regex_search(p, m, reCommon)) {
    return false;
  }
  const glm::vec2 reciprocalTexSize(1.0f / atoi(m[1].first), 1.0f / atoi(m[2].first));
  p = m[0].second;

  if (!std::regex_search(p, m, rePage)) {
    return false;
  }
  texFilename.assign(m[1].first, m[1].second);
  if (!GameEngine::Instance().LoadTextureFromFile(texFilename.c_str())) {
    return false;
  }
  p = m[0].second;

  if (!std::regex_search(p, m, reChars)) {
    return false;
  }
  p = m[0].second;
  fontList.resize(128);

  const std::cregex_iterator end;
  for (std::cregex_iterator itr(p, buffer.data() + buffer.size(), reChar); itr != end; ++itr) {
    const auto& m = *itr;
    Font font;
    font.id = atoi(m[1].first);
    font.uv.x = atof(m[2].first);
    font.uv.y = atof(m[3].first);
    font.uv *= reciprocalTexSize;
    font.size.x = atof(m[4].first);
    font.size.y = atof(m[5].first);
    font.size *= reciprocalTexSize;
    font.uv.y = 1 - font.uv.y - font.size.y;
    font.offset.x = atof(m[6].first);
    font.offset.y = atof(m[7].first);
    font.offset *= reciprocalTexSize;
    font.xadvance = atof(m[8].first) * reciprocalTexSize.x;
    if (font.id < 128) {
      fontList[font.id] = font;
    }
  }

  return true;
}

/**
* �������ǉ�����.
*
* @param position �\���J�n���W.
* @param str      �ǉ����镶����.
*
* @retval true  �ǉ�����.
* @retval false �ǉ����s.
*/
bool Renderer::AddString(const glm::vec2& position, const char* str)
{
  const glm::u16vec2 thicknessAndOutline = glm::vec2(0.625f - thickness * 0.375f, border) * 65535.0f;
  Vertex* pp = pVBO + vboSize;
  glm::vec2 pos = position;
  for (const char* itr = str; *itr; ++itr) {
    if (vboSize + 4 > vboCapacity) {
      break;
    }
    const Font& font = fontList[*itr];
    if (font.id < 0) {
      continue;
    }
    if (font.size.x != font.size.y) {
      const glm::vec2 size = font.size * scale;
      const glm::vec2 offsetedPos = pos + font.offset * scale;
      Vertex p[4];
      p[0].position = offsetedPos;
      p[0].uv = font.uv * 65535.0f;
      p[0].color = color;
      p[0].subColor = subColor;
      p[0].thicknessAndOutline = thicknessAndOutline;

      p[1].position = offsetedPos + glm::vec2(size.x, 0);
      p[1].uv = (font.uv + glm::vec2(font.size.x, 0))  * 65535.0f;
      p[1].color = color;
      p[1].subColor = subColor;
      p[1].thicknessAndOutline = thicknessAndOutline;

      p[2].position = offsetedPos + size;
      p[2].uv = (font.uv + font.size) * 65535.0f;
      p[2].color = color;
      p[2].subColor = subColor;
      p[2].thicknessAndOutline = thicknessAndOutline;

      p[3].position = offsetedPos + glm::vec2(0, size.y);
      p[3].uv = (font.uv + glm::vec2(0, font.size.y)) * 65535.0f;
      p[3].color = color;
      p[3].subColor = subColor;
      p[3].thicknessAndOutline = thicknessAndOutline;

      memcpy(pp, p, sizeof(p));
      pp += 4;
      vboSize += 4;
    }
    pos.x += font.xadvance * scale.x;
  }
  return true;
}

/**
* VBO���V�X�e���������Ƀ}�b�s���O����.
*/
void Renderer::MapBuffer()
{
  if (pVBO && vboSize == 0) {
    return;
  }
  glBindBuffer(GL_ARRAY_BUFFER, vbo.Id());
  pVBO = static_cast<Vertex*>(glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * vboCapacity, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
  if (!pVBO) {
    const GLenum err = glGetError();
    std::cerr << "ERROR: MapBuffer���s(0x" << std::hex << err << ")" << std::endl;
  }
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  vboSize = 0;
}

/**
* VBO�̃}�b�s���O����������.
*/
void Renderer::UnmapBuffer()
{
  if (pVBO && vboSize == 0) {
    return;
  }
  glBindBuffer(GL_ARRAY_BUFFER, vbo.Id());
  glUnmapBuffer(GL_ARRAY_BUFFER);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/**
* �t�H���g��`�悷��.
*/
void Renderer::Draw() const
{
  if (vboSize > 0) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    vao.Bind();
    progFont->UseProgram();
    progFont->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, GameEngine::Instance().GetTexture(texFilename.c_str())->Id());
    glDrawElements(GL_TRIANGLES, (vboSize / 4) * 6, GL_UNSIGNED_SHORT, 0);
    vao.Unbind();
  }
}

} // namespace Font