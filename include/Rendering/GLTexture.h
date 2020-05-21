#pragma once
#include <Windows.h>
#include <gl/glew.h>
#include <SDL2/SDL_image.h>


static GLenum GetScaleQuality()
{
  const char* hint = SDL_GetHint(SDL_HINT_RENDER_SCALE_QUALITY);

  if (!hint || *hint == '0' || SDL_strcasecmp(hint, "nearest") == 0) {
    return GL_NEAREST;
  }
  else {
    return GL_LINEAR;
  }
}

// binds a SDL_Surface to a gl id
struct GLTexture
{
  Uint8 colors;
  GLuint id;
  
  int width;
  int height;
  GLfloat w;
  GLfloat h;
  SDL_BlendMode blendMode;

  GLenum type;
  GLenum textureFormat;
  GLuint internalFormat;

  GLTexture(SDL_Surface* surface) : type(GL_UNSIGNED_BYTE), textureFormat(GL_RGBA), internalFormat(GL_RGBA8)
  {
    width = surface->w;
    height = surface->h;

    w = (GLfloat)width;
    h = (GLfloat)height;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, surface->w, surface->h, 0, textureFormat, GL_UNSIGNED_BYTE, surface->pixels);

    GLenum scaleMode = GetScaleQuality();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, scaleMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, scaleMode);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  ~GLTexture()
  {
    glDeleteTextures(1, &id);
  }

  void SetFormat(Uint32 format)
  {
    switch (format) {
    case SDL_PIXELFORMAT_ARGB8888:
      internalFormat = GL_RGBA8;
      textureFormat = GL_BGRA;
      type = GL_UNSIGNED_INT_8_8_8_8_REV;
      break;
    case SDL_PIXELFORMAT_YV12:
    case SDL_PIXELFORMAT_IYUV:
      internalFormat = GL_LUMINANCE;
      textureFormat = GL_LUMINANCE;
      type = GL_UNSIGNED_BYTE;
      break;
    default:
      return;
    }
  }
};

struct GLTextureEx
{
  GLuint id;
  

  GLTextureEx() : type(GL_UNSIGNED_BYTE), format(GL_RGBA)
  {}

  GLenum type;
  GLenum format;

  GLenum formattype;
  void* pixels;
  int pitch;
  SDL_Rect locked_rect;
};