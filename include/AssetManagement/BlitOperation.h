#pragma once
#include "AssetManagement/Resource.h"
#include "AssetManagement/Text.h"

typedef Resource<SDL_Texture> Texture;
typedef Resource<TTF_Font> Font;

//! Drawing parameters drawing sprite objects
struct BlitOperation
{
  bool valid = false;
  SDL_Rect _textureRect;
  SDL_Rect _displayRect;
  Texture* _textureResource;
  SDL_RendererFlip _flip;
  SDL_Color _displayColor;
};
