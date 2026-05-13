#pragma once
#include <bgfx/bgfx.h>
#include <SDL2/SDL_image.h>
#include <string>

class BgfxTexture
{
public:
  // Call before bgfx::shutdown() to prevent late destructors from crashing
  static void SetBgfxShutdown() { s_bgfxAlive = false; }

  BgfxTexture();
  BgfxTexture(BgfxTexture&& other) noexcept;
  BgfxTexture& operator=(BgfxTexture&& other) noexcept;
  ~BgfxTexture() noexcept;

  void LoadFromFile(const std::string& fileName);
  void LoadFromSurface(SDL_Surface* surface);
  void CreateEmpty(int width, int height, Uint32 format);
  void Update(void* pixels);

  bgfx::TextureHandle Handle() const { return _handle; }
  int w() const { return _w; }
  int h() const { return _h; }

private:
  BgfxTexture(const BgfxTexture&) = delete;
  BgfxTexture& operator=(const BgfxTexture&) = delete;

  void CreateFromSurface(SDL_Surface* surface);

  bgfx::TextureHandle _handle;
  int _w;
  int _h;
  bgfx::TextureFormat::Enum _format;

  static bool s_bgfxAlive;
};
