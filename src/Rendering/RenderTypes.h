#pragma once
#include <cstdint>

struct Color
{
  uint8_t r, g, b, a;
};

enum class FlipMode : uint8_t
{
  None       = 0,
  Horizontal = 1,
  Vertical   = 2,
  Both       = 3
};

// SDL conversion helpers — only available in translation units that include SDL
#ifdef SDL_h_
inline SDL_Color ToSDLColor(const Color& c) { return { c.r, c.g, c.b, c.a }; }
inline Color FromSDLColor(const SDL_Color& c) { return { c.r, c.g, c.b, c.a }; }
inline SDL_RendererFlip ToSDLFlip(FlipMode f)
{
  switch (f)
  {
    case FlipMode::Horizontal: return SDL_FLIP_HORIZONTAL;
    case FlipMode::Vertical:   return SDL_FLIP_VERTICAL;
    case FlipMode::Both:       return static_cast<SDL_RendererFlip>(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
    default:                   return SDL_FLIP_NONE;
  }
}
#endif
