#pragma once
#include "AssetManagement/Resource.h"
#include "AssetManagement/Text.h"

#include "Core/Geometry2D/Rect.h"

struct RenderCommand
{
  bool valid = false;
  DrawRect<float> targetRect;
  SDL_Color displayColor = SDL_Color{ 0, 0, 0, 0 };
  SDL_RendererFlip flip = SDL_FLIP_NONE;
};

//! Drawing parameters drawing sprite objects
template <typename AssetType>
struct BlitOperation : public RenderCommand
{
  DrawRect<float> srcRect;
  Resource<AssetType>* textureResource = nullptr;
};

//!
template <typename AssetType>
struct DrawPrimitive : public RenderCommand
{
  //! rect bounds in world space
  bool filled = false;

};

struct RenderCube
{
  bool valid = false;
  SDL_Color faceColor[6];
};

struct RenderTriangle
{
  bool valid = false;
  SDL_Color vertColors[3];
};
