#include "Rendering/BgfxTexture.h"
#include <memory>
#include <stdexcept>

bool BgfxTexture::s_bgfxAlive = true;

BgfxTexture::BgfxTexture()
  : _handle(BGFX_INVALID_HANDLE)
  , _w(0)
  , _h(0)
  , _format(bgfx::TextureFormat::RGBA8)
{}

BgfxTexture::BgfxTexture(BgfxTexture&& other) noexcept
  : _handle(other._handle)
  , _w(other._w)
  , _h(other._h)
  , _format(other._format)
{
  other._handle = BGFX_INVALID_HANDLE;
  other._w = 0;
  other._h = 0;
}

BgfxTexture& BgfxTexture::operator=(BgfxTexture&& other) noexcept
{
  if (this != &other)
  {
    if (s_bgfxAlive && bgfx::isValid(_handle))
      bgfx::destroy(_handle);

    _handle = other._handle;
    _w = other._w;
    _h = other._h;
    _format = other._format;

    other._handle = BGFX_INVALID_HANDLE;
    other._w = 0;
    other._h = 0;
  }
  return *this;
}

BgfxTexture::~BgfxTexture() noexcept
{
  if (s_bgfxAlive && bgfx::isValid(_handle))
    bgfx::destroy(_handle);
}

void BgfxTexture::LoadFromFile(const std::string& fileName)
{
  if (bgfx::isValid(_handle))
  {
    bgfx::destroy(_handle);
    _handle = BGFX_INVALID_HANDLE;
  }

  std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)> surface(IMG_Load(fileName.c_str()), SDL_FreeSurface);
  if (!surface)
    throw std::invalid_argument("Could not load texture data from file " + fileName);

  CreateFromSurface(surface.get());
}

void BgfxTexture::LoadFromSurface(SDL_Surface* surface)
{
  if (bgfx::isValid(_handle))
  {
    bgfx::destroy(_handle);
    _handle = BGFX_INVALID_HANDLE;
  }

  CreateFromSurface(surface);
}

void BgfxTexture::CreateEmpty(int width, int height, Uint32 format)
{
  if (bgfx::isValid(_handle))
  {
    bgfx::destroy(_handle);
    _handle = BGFX_INVALID_HANDLE;
  }

  _w = width;
  _h = height;
  _format = bgfx::TextureFormat::RGBA8;

  _handle = bgfx::createTexture2D(
    uint16_t(_w), uint16_t(_h), false, 1,
    _format,
    BGFX_TEXTURE_NONE | BGFX_SAMPLER_POINT
  );
}

void BgfxTexture::Update(void* pixels)
{
  if (!bgfx::isValid(_handle))
    return;

  uint32_t pitch = _w * 4; // RGBA8
  const bgfx::Memory* mem = bgfx::copy(pixels, pitch * _h);
  bgfx::updateTexture2D(_handle, 0, 0, 0, 0, uint16_t(_w), uint16_t(_h), mem, uint16_t(pitch));
}

void BgfxTexture::CreateFromSurface(SDL_Surface* surface)
{
  // Convert to RGBA32 for consistent format
  SDL_Surface* converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
  if (!converted)
    throw std::invalid_argument("Could not convert surface to RGBA32");

  _w = converted->w;
  _h = converted->h;
  _format = bgfx::TextureFormat::RGBA8;

  uint32_t pitch = converted->pitch;
  const bgfx::Memory* mem = bgfx::copy(converted->pixels, pitch * _h);

  _handle = bgfx::createTexture2D(
    uint16_t(_w), uint16_t(_h), false, 1,
    _format,
    BGFX_TEXTURE_NONE | BGFX_SAMPLER_POINT,
    mem
  );

  SDL_FreeSurface(converted);
}
