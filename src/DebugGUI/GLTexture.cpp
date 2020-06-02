#include "DebugGUI/GLTexture.h"
#include "GameManagement.h"

//______________________________________________________________________________
GLTexture::GLTexture(GLTexture&& other) noexcept :
  mTextureName(std::move(other.mTextureName)), mTextureId(other.mTextureId)
{
  other.mTextureId = 0;
}

//______________________________________________________________________________
GLTexture& GLTexture::operator=(GLTexture&& other) noexcept
{
  mTextureName = std::move(other.mTextureName);
  std::swap(mTextureId, other.mTextureId);
  return *this;
}

//______________________________________________________________________________
GLTexture::~GLTexture() noexcept
{
  if (mTextureId)
    glDeleteTextures(1, &mTextureId);
}

//______________________________________________________________________________
GLTexture::Handle GLTexture::LoadFromFile(const std::string& fileName)
{
  if (mTextureId)
  {
    glDeleteTextures(1, &mTextureId);
    mTextureId = 0;
  }

  Handle textureData(IMG_Load(fileName.c_str()), SDL_FreeSurface);
  if (!textureData)
    throw std::invalid_argument("Could not load texture data from file " + fileName);
  SetTextureParameters(textureData.get());

  return textureData;
}

//______________________________________________________________________________
GLTexture::Handle GLTexture::CreateEmpty(int width, int height)
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

  SDL_Surface* unformattedSurface = SDL_CreateRGBSurface(0, width, height, 32, rmask, gmask, bmask, amask);
  SDL_Surface* surface = SDL_ConvertSurfaceFormat(unformattedSurface, GameManager::Get().GetWindowFormat(), 0);
  // Set blend mode for alpha blending
  if (SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND) != 0)
    throw std::invalid_argument("Could not create texture data from surface");

  SDL_FreeSurface(unformattedSurface);

  Handle textureData(surface, SDL_FreeSurface);
  if (!textureData)
    throw std::invalid_argument("Could not create texture data from surface");
  SetTextureParameters(textureData.get());

  return textureData;
}

//______________________________________________________________________________
void GLTexture::Update(void* pixels)
{
  glBindTexture(GL_TEXTURE_2D, mTextureId);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glColor4f(1.0, 1.0, 1.0, 1.0); 

  glTexImage2D(GL_TEXTURE_2D, 0, nColors, _w, _h, 0, format, GL_UNSIGNED_BYTE, pixels);
}

//______________________________________________________________________________
void GLTexture::SetTextureParameters(SDL_Surface* textureData)
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

  glColor4f(1.0, 1.0, 1.0, 1.0); 

  _w = textureData->w;
  _h = textureData->h;

  glTexImage2D(GL_TEXTURE_2D, 0, nColors, textureData->w, textureData->h, 0, format, GL_UNSIGNED_BYTE, textureData->pixels);
}
