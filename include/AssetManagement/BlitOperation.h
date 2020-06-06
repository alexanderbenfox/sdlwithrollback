#pragma once
#include "AssetManagement/Resource.h"
#include "AssetManagement/Text.h"

//! Drawing parameters drawing sprite objects
template <typename AssetType>
struct BlitOperation
{
  bool valid = false;
  SDL_Rect _textureRect = { 0, 0, 0, 0 };
  SDL_Rect _displayRect = { 0, 0, 0, 0 };
  Resource<AssetType>* _textureResource = nullptr;
  SDL_RendererFlip _flip = SDL_FLIP_NONE;
  SDL_Color _displayColor = SDL_Color{ 0, 0, 0, 0 };
};
