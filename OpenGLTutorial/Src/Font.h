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

/// フォント情報.
struct FontInfo {
  int id = -1;        ///< 文字コード.
  glm::u16vec2 uv[2]; ///< フォント画像のテクスチャ座標.
  glm::vec2 size;     ///< フォント画像の表示サイズ.
  glm::vec2 offset;   ///< 表示位置をずらす距離.
  float xadvance = 0; ///< カーソルを進める距離.
};

/**
* ビットマップフォント描画クラス.
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
  GLsizei vboCapacity = 0;        ///< VBOに格納可能な最大頂点数.
  std::vector<FontInfo> fontList; ///< フォント位置情報のリスト.
  std::string texFilename;        ///< フォントテクスチャファイル名.
  Shader::ProgramPtr progFont;    ///< フォント描画用シェーダプログラム.
  glm::vec2 reciprocalScreenSize; ///< 画面サイズの逆数.

  glm::vec2 scale = glm::vec2(1, 1); ///< フォントを描画するときの拡大率.
  glm::u8vec4 color = glm::u8vec4(255, 255, 255, 255); ///< フォントを描画するときの色.
  GLsizei vboSize = 0;            ///< VBOに格納されている頂点数.
  Vertex* pVBO = nullptr;         ///< VBOへのポインタ.
};

} // namespace Font

#endif // FONT_H_INCLUDED
