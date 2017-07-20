/**
* @file Font.cpp
*/
#include "Font.h"
#include "GameEngine.h"
#include <memory>
#include <iostream>
#include <stdio.h>

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
* @param screen  �`���X�N���[���̑傫��.
*
* @retval true  ����������.
* @retval false ���������s.
*/
bool Renderer::Init(size_t maxChar, const glm::vec2& screen)
{
  if (maxChar > (USHRT_MAX + 1) / 4) {
    std::cerr << "WARNING: " << maxChar << "�͐ݒ�\�ȍő啶�������z���Ă��܂�"<< std::endl;
    maxChar = (USHRT_MAX + 1) / 4;
  }
  vboCapacity = static_cast<GLsizei>(4 * maxChar);
  vbo.Init(GL_ARRAY_BUFFER, sizeof(Vertex) * vboCapacity, nullptr, GL_STREAM_DRAW);
  {
    std::vector<GLushort> tmp;
    tmp.resize(maxChar * 6);
    GLushort* p = tmp.data();
    for (GLushort i = 0; i < maxChar * 4; i += 4) {
      for (GLshort n : { 0, 1, 2, 2, 3, 0 }) {
        *(p++) = i + n;
      }
    }
    ibo.Init(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 6 * maxChar, tmp.data(), GL_STATIC_DRAW);
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

  reciprocalScreenSize = 1.0f / screen;
  return true;
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
  const std::unique_ptr<FILE, decltype(&fclose)> fp(fopen(filename, "r"), fclose);
  if (!fp) {
    return false;
  }

  int line = 1;
  float fontSize;
  int ret = fscanf(fp.get(), "info face=%*s size=%f bold=%*d italic=%*d charset=%*s"
    " unicode=%*d stretchH=%*d smooth=%*d aa=%*d padding=%*d,%*d,%*d,%*d spacing=%*d,%*d", &fontSize);
  ++line;
  const float reciprocalFontSize = 1.0f / fontSize;

  glm::vec2 scale;
  float ybase;
  ret = fscanf(fp.get(), " common lineHeight=%*d base=%f scaleW=%f scaleH=%f pages=%*d packed=%*d", &ybase, &scale.x, &scale.y);
  if (ret < 2) {
    std::cerr << "ERROR: '" << filename << "'�̓ǂݍ��݂Ɏ��s(line=" << line << ")" << std::endl;
    return false;
  }
  reciprocalTexSize = glm::vec2(1.0f / scale);
  ++line;

  char tex[128];
  ret = fscanf(fp.get(), " page id=%*d file=%127s", tex);
  if (ret < 1) {
    std::cerr << "ERROR: '" << filename << "'�̓ǂݍ��݂Ɏ��s(line=" << line << ")" << std::endl;
    return false;
  }
  texFilename.assign(tex + 1, tex + strlen(tex) - 1);
  ++line;

  int charCount;
  ret = fscanf(fp.get(), " chars count=%d", &charCount);
  if (ret < 1) {
    std::cerr << "ERROR: '" << filename << "'�̓ǂݍ��݂Ɏ��s(line=" << line << ")" << std::endl;
    return false;
  }
  ++line;

  fixedAdvance = 0;
  fontList.resize(128);
  for (int i = 0; i < charCount; ++i) {
    FontInfo font;
    ret = fscanf(fp.get(), " char id=%d x=%f y=%f width=%f height=%f xoffset=%f yoffset=%f xadvance=%f page=%*d chnl=%*d", &font.id, &font.uv.x, &font.uv.y, &font.orgSize.x, &font.orgSize.y, &font.offset.x, &font.offset.y, &font.xadvance);
    if (ret < 8) {
      std::cerr << "ERROR: '" << filename << "'�̓ǂݍ��݂Ɏ��s(line=" << line << ")" << std::endl;
      return false;
    }
    font.uv *= reciprocalTexSize;
    font.tsize = font.orgSize * reciprocalTexSize;
    font.uv.y = 1 - font.uv.y - font.tsize.y;
    font.offset.y *= -1;
    if (font.id < 128) {
      fontList[font.id] = font;
      if (font.xadvance > fixedAdvance) {
        fixedAdvance = font.xadvance;
      }
    }
    ++line;
  }
  GameEngine& game = GameEngine::Instance();
  if (!game.LoadTextureFromFile(texFilename.c_str())) {
    return false;
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

  Vertex* p = pVBO + vboSize;
  glm::vec2 pos = position;
  for (const char* itr = str; *itr; ++itr) {
    if (vboSize + 4 > vboCapacity) {
      break;
    }
    const FontInfo& font = fontList[*itr];
    if (font.id >= 0 && font.orgSize.x && font.orgSize.y) {
      const glm::vec2 ssize = font.orgSize * reciprocalScreenSize * scale;
      const glm::vec2 offsetedPos = propotional ? pos + (font.offset * reciprocalScreenSize) * scale : pos;
      p[0].position = offsetedPos + glm::vec2(0, -ssize.y);
      p[0].uv = font.uv * 65535.0f;
      p[0].color = color;
      p[0].subColor = subColor;
      p[0].thicknessAndOutline = thicknessAndOutline;

      p[1].position = offsetedPos + glm::vec2(ssize.x, -ssize.y);
      p[1].uv = (font.uv + glm::vec2(font.tsize.x, 0))  * 65535.0f;
      p[1].color = color;
      p[1].subColor = subColor;
      p[1].thicknessAndOutline = thicknessAndOutline;

      p[2].position = offsetedPos + glm::vec2(ssize.x, 0);
      p[2].uv = (font.uv + font.tsize) * 65535.0f;
      p[2].color = color;
      p[2].subColor = subColor;
      p[2].thicknessAndOutline = thicknessAndOutline;

      p[3].position = offsetedPos;
      p[3].uv = (font.uv + glm::vec2(0, font.tsize.y)) * 65535.0f;
      p[3].color = color;
      p[3].subColor = subColor;
      p[3].thicknessAndOutline = thicknessAndOutline;

      p += 4;
      vboSize += 4;
    }
    pos.x += (propotional ? (font.xadvance * reciprocalScreenSize.x) : fixedAdvance) * scale.x;
  }
  return true;
}

/**
* �����F��ݒ肷��.
*
* @param c �����F.
*/
void Renderer::Color(const glm::vec4& c)
{
  color = glm::clamp(c, 0.0f, 1.0f) * 255.0f;
}

/**
* �����F�擾����.
*
* @return �����F.
*/
glm::vec4 Renderer::Color() const
{
  return glm::vec4(color) * (1.0f / 255.0f);
}

/**
* �T�u�����F��ݒ肷��.
*
* @param c �����F.
*/
void Renderer::SubColor(const glm::vec4& c)
{
  subColor = glm::clamp(c, 0.0f, 1.0f) * 255.0f;
}

/**
* �T�u�����F�擾����.
*
* @return �����F.
*/
glm::vec4 Renderer::SubColor() const
{
  return glm::vec4(subColor) * (1.0f / 255.0f);
}

/**
* VBO���V�X�e���������Ƀ}�b�s���O����.
*/
void Renderer::MapBuffer()
{
  if (pVBO) {
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
  if (!pVBO || vboSize == 0) {
    return;
  }
  glBindBuffer(GL_ARRAY_BUFFER, vbo.Id());
  glUnmapBuffer(GL_ARRAY_BUFFER);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  pVBO = nullptr;
}

/**
* �t�H���g��`�悷��.
*/
void Renderer::Draw() const
{
  if (vboSize > 0) {
    vao.Bind();
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    progFont->UseProgram();
    progFont->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, GameEngine::Instance().GetTexture(texFilename.c_str())->Id());
    glDrawElements(GL_TRIANGLES, (vboSize / 4) * 6, GL_UNSIGNED_SHORT, 0);
    vao.Unbind();
  }
}

} // namespace Font