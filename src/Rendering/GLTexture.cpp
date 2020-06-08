#include "Rendering/GLTexture.h"
#include <memory>
#include <stdexcept>

//______________________________________________________________________________
GLTexture::GLTexture(GLTexture&& other) noexcept : _textureId(other._textureId)
{
  other._textureId = 0;
}

//______________________________________________________________________________
GLTexture& GLTexture::operator=(GLTexture&& other) noexcept
{
  std::swap(_textureId, other._textureId);
  return *this;
}

//______________________________________________________________________________
GLTexture::~GLTexture() noexcept
{
  if (_textureId)
    glDeleteTextures(1, &_textureId);
}

//______________________________________________________________________________
void GLTexture::LoadFromFile(const std::string& fileName)
{
  if (_textureId)
  {
    glDeleteTextures(1, &_textureId);
    _textureId = 0;
  }

  std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)>textureData(IMG_Load(fileName.c_str()), SDL_FreeSurface);
  if (!textureData)
    throw std::invalid_argument("Could not load texture data from file " + fileName);
  SetTextureParameters(textureData.get());
}

//______________________________________________________________________________
void GLTexture::LoadFromSurface(SDL_Surface* surface)
{
  if (_textureId)
  {
    glDeleteTextures(1, &_textureId);
    _textureId = 0;
  }

  SetTextureParameters(surface);
}

//______________________________________________________________________________
void GLTexture::CreateEmpty(int width, int height, Uint32 format)
{
  if (_textureId)
  {
    glDeleteTextures(1, &_textureId);
    _textureId = 0;
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

  SDL_Surface* unformattedSurface = SDL_CreateRGBSurface(0, width, height, 32, rmask, gmask, bmask, amask);
  SDL_Surface* surface = SDL_ConvertSurfaceFormat(unformattedSurface, format, 0);
  // Set blend mode for alpha blending
  if (SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND) != 0)
    throw std::invalid_argument("Could not create texture data from surface");

  SDL_FreeSurface(unformattedSurface);

  std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)> textureData(surface, SDL_FreeSurface);
  if (!textureData)
    throw std::invalid_argument("Could not create texture data from surface");
  SetTextureParameters(textureData.get());
}

//______________________________________________________________________________
void GLTexture::Update(void* pixels)
{
  glBindTexture(GL_TEXTURE_2D, _textureId);

  GLenum scaleMode = GetScaleQuality();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, scaleMode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, scaleMode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glColor4f(1.0, 1.0, 1.0, 1.0);

  glTexImage2D(GL_TEXTURE_2D, 0, _internalFormat, _w, _h, 0, _textureFormat, _type, pixels);
}

//______________________________________________________________________________
void GLTexture::SetTextureParameters(SDL_Surface* textureData)
{
  Uint8 nChannels = textureData->format->BytesPerPixel;
  if (nChannels == 4)
  {
    if (textureData->format->Rmask == 0x000000FF)
      _textureFormat = GL_RGBA;
    else
      _textureFormat = GL_BGRA_EXT;
    _internalFormat = GL_RGBA8;
  }
  else if (nChannels == 3)
  {
    if (textureData->format->Rmask == 0x000000ff)
      _textureFormat = GL_RGB;
    else
      _textureFormat = GL_BGR_EXT;
    _internalFormat = GL_RGB8;
  }

  _type = GL_UNSIGNED_BYTE;

  SDL_GetSurfaceBlendMode(textureData, &_blendMode);


  // set up the gl texture after  the internal format has been parsed
  glGenTextures(1, &_textureId);
  glBindTexture(GL_TEXTURE_2D, _textureId);

  GLenum scaleMode = GetScaleQuality();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, scaleMode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, scaleMode);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glColor4f(1.0, 1.0, 1.0, 1.0);

  _w = textureData->w;
  _h = textureData->h;

  glTexImage2D(GL_TEXTURE_2D, 0, _internalFormat, textureData->w, textureData->h, 0, _textureFormat, _type, textureData->pixels);
}
