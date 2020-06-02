#pragma once

#include <cstdint>
#include <memory>
#include <utility>
#include <string>
#include <iostream>

#if defined(_WIN32)
#if defined(APIENTRY)
#undef APIENTRY
#endif
#include <windows.h>
#include <GL/GL.h>
#include <GL/GLU.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#endif

#include <SDL2_image/SDL_image.h>

// gl texture class for showing textures in debug ui
class GLTexture
{
public:
  //! texture handler
  using Handle = std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)>;

  //! default constructor
  GLTexture() = default;
  //! constructor setting texutre name
  GLTexture(const std::string& textureName) : mTextureName(textureName), mTextureId(0) {}
  //! constructor with specific texture id
  GLTexture(const std::string& textureName, GLint textureId) : mTextureName(textureName), mTextureId(textureId) {}

  //! rvalue copy constructor
  GLTexture(GLTexture&& other) noexcept;
  //! rvalue assignment operator
  GLTexture& operator=(GLTexture&& other) noexcept;
  //! deconstuctor removes texture from gl context
  ~GLTexture() noexcept;

  //! Load a file in memory and create an OpenGL texture.
  Handle LoadFromFile(const std::string& fileName);

  Handle CreateEmpty(int width, int height);

  void Update(void* pixels);

  //! Getters
  GLuint texture() const { return mTextureId; }
  const std::string& textureName() const { return mTextureName; }

private:
  //! delete copy constructor and assignment operator
  GLTexture(const GLTexture& other) = delete;
  GLTexture& operator=(const GLTexture& other) = delete;
  
  //! Sets GL parameters based on SDL_Surface
  void SetTextureParameters(SDL_Surface* textureData);

  std::string mTextureName;
  GLuint mTextureId;
  int _w, _h;
  GLint format;
  GLint nColors;

};
