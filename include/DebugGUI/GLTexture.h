#pragma once

#include <cstdint>
#include <memory>
#include <utility>
#include <string>
#include <iostream>

#if defined(_WIN32)
#  if defined(APIENTRY)
#    undef APIENTRY
#  endif
#  include <windows.h>
#endif

#include <GL/GL.h>
#include <GL/GLU.h>
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
    
    GLint format;
    GLint nColors = textureData->format->BytesPerPixel;
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

    glTexImage2D(GL_TEXTURE_2D, 0, nColors, textureData->w, textureData->h, 0, format, GL_UNSIGNED_BYTE, textureData->pixels);

    return textureData;
  }

private:
  std::string mTextureName;
  GLuint mTextureId;
};