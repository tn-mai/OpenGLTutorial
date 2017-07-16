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

struct Font {
  int id = -1;
  glm::vec2 uv;
  glm::vec2 size;
  glm::vec2 offset;
  float xadvance = 0;
};

class Renderer
{
public:
  Renderer() = default;
  ~Renderer();
  Renderer(const Renderer&) = default;
  Renderer& operator=(const Renderer&) = default;

  bool Init(size_t maxChar, const glm::vec2& ss);
  bool LoadFromFile(const char* filename);
  void Scale(const glm::vec2& s) { scale = s; }
  const glm::vec2& Scale() const { return scale; }
  void Color(glm::u8vec4 c) { color = c; }
  glm::u8vec4 Color() const { return color; }
  void SubColor(glm::u8vec4 c) { subColor = c; }
  glm::u8vec4 SubColor() const { return subColor; }
  void Thickness(float t) { thickness = t; }
  float Thickness() const { return thickness; }
  void Border(float b) { border = b; }
  float Border() const { return border; }

  bool AddString(const glm::vec2& position, const char* str);
  void MapBuffer();
  void UnmapBuffer();
  void Draw() const;

private:
  BufferObject vbo;
  BufferObject ibo;
  VertexArrayObject vao;
  GLsizei vboCapacity = 0;
  std::vector<Font> fontList;
  std::string texFilename;
  Shader::ProgramPtr progFont;
  glm::vec2 screenSize;
  glm::vec2 screenScale;

  glm::vec2 scale = glm::vec2(1, 1);
  glm::u8vec4 color = glm::u8vec4(255, 255, 255, 255);
  glm::u8vec4 subColor = glm::u8vec4(64, 64, 64, 255);
  float thickness = 0.5f;
  float border = 0;

  GLsizei vboSize = 0;
  Vertex* pVBO = nullptr;
};

} // namespace Font

#endif // FONT_H_INCLUDED