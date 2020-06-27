#pragma once
#include "AssetManagement/Resource.h"
#include "AssetManagement/Text.h"

struct RenderCommand
{
  bool valid = false;
  SDL_Rect displayRect = { 0, 0, 0, 0 };
  SDL_Color displayColor = SDL_Color{ 0, 0, 0, 0 };
  SDL_RendererFlip flip = SDL_FLIP_NONE;
};

//! Drawing parameters drawing sprite objects
template <typename AssetType>
struct BlitOperation : public RenderCommand
{
  SDL_Rect textureRect = { 0, 0, 0, 0 };
  Resource<AssetType>* textureResource = nullptr;
};

//!
template <typename AssetType>
struct DrawPrimitive : public RenderCommand
{
  //! rect bounds in world space
  bool filled = false;

};
