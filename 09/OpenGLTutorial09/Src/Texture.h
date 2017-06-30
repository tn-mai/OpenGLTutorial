/**
* @file Texture.h
*/
#ifndef OPENGLTUTORIAL_SRC_TEXTURE_H_INCLUDED
#define OPENGLTUTORIAL_SRC_TEXTURE_H_INCLUDED
#include <GL/glew.h>
#include <memory>

class Texture;
typedef std::shared_ptr<Texture> TexturePtr;

/**
* テクスチャクラス.
*/
class Texture
{
public:
  static TexturePtr Create(int width, int height, GLenum iformat, GLenum format, const void* data, GLenum wrapMode = GL_CLAMP_TO_EDGE);
  static TexturePtr LoadFromFile(const char*, GLenum wrapMode = GL_CLAMP_TO_EDGE);

  GLuint Id() const { return texId; }
  GLsizei Width() const { return width; }
  GLsizei Height() const { return height; }

private:
  Texture();
  ~Texture();
  Texture(const Texture&) = delete;
  Texture& operator=(const Texture&) = delete;

  GLuint texId;
  int width;
  int height;
};

#endif // OPENGLTUTORIAL_SRC_TEXTURE_H_INCLUDED