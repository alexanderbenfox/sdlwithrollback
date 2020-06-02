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
  using handleType = std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)>;
  GLTexture() = default;
  GLTexture(const std::string& textureName)
    : mTextureName(textureName), mTextureId(0) {}

  GLTexture(const std::string& textureName, GLint textureId)
    : mTextureName(textureName), mTextureId(textureId) {}

  GLTexture(const GLTexture& other) = delete;
  GLTexture(GLTexture&& other) noexcept
    : mTextureName(std::move(other.mTextureName)),
    mTextureId(other.mTextureId) {
    other.mTextureId = 0;
  }
  GLTexture& operator=(const GLTexture& other) = delete;
  GLTexture& operator=(GLTexture&& other) noexcept {
    mTextureName = std::move(other.mTextureName);
    std::swap(mTextureId, other.mTextureId);
    return *this;
  }
  ~GLTexture() noexcept {
    if (mTextureId)
      glDeleteTextures(1, &mTextureId);
  }

  GLuint texture() const { return mTextureId; }
  const std::string& textureName() const { return mTextureName; }

  /**
  *  Load a file in memory and create an OpenGL texture.
  *  Returns a handle type (an std::unique_ptr) to the loaded pixels.
  */
  handleType load(const std::string& fileName)
  {
    if (mTextureId)
    {
      glDeleteTextures(1, &mTextureId);
      mTextureId = 0;
    }

    handleType textureData(IMG_Load(fileName.c_str()), SDL_FreeSurface);
    if (!textureData)
      throw std::invalid_argument("Could not load texture data from file " + fileName);
    setupGLTexture(textureData.get());

    return textureData;
  }

  handleType create(int width, int height)
  {
    if (mTextureId)
    {
      glDeleteTextures(1, &mTextureId);
      mTextureId = 0;
    }

    Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    handleType textureData(SDL_CreateRGBSurface(0, width, height, 32, rmask, gmask, bmask, amask), SDL_FreeSurface);
    if (!textureData)
      throw std::invalid_argument("Could not create texture data from surface");
    setupGLTexture(textureData.get());

    return textureData;
  }

  void setupGLTexture(SDL_Surface* textureData)
  {
    nColors = textureData->format->BytesPerPixel;
    if (nColors == 4)
    {
      if (textureData->format->Rmask == 0x000000FF)
        format = GL_RGBA;
      else
        format = GL_BGRA_EXT;
    }
    else if (nColors == 3)
    {
      if (textureData->format->Rmask == 0x000000ff)
        format = GL_RGB;
      else
        format = GL_BGR_EXT;
    }

    glGenTextures(1, &mTextureId);
    glBindTexture(GL_TEXTURE_2D, mTextureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    _w = textureData->w;
    _h = textureData->h;

    glTexImage2D(GL_TEXTURE_2D, 0, nColors, textureData->w, textureData->h, 0, format, GL_UNSIGNED_BYTE, textureData->pixels);
  }

  void update(void* pixels)
  {
    glTexImage2D(GL_TEXTURE_2D, 0, nColors, _w, _h, 0, format, GL_UNSIGNED_BYTE, pixels);
  }

private:
  std::string mTextureName;
  GLuint mTextureId;
  int _w, _h;
  GLint format;
  GLint nColors;
};