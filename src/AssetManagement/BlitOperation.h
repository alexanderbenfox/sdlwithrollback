#pragma once
#include "AssetManagement/Resource.h"
#include "Rendering/RenderTypes.h"

#include "Core/Geometry2D/Rect.h"

struct RenderCommand
{
  bool valid = false;
  DrawRect<float> targetRect;
  Color displayColor = Color{ 0, 0, 0, 0 };
  FlipMode flip = FlipMode::None;
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
  Color faceColor[6];
};

struct RenderTriangle
{
  bool valid = false;
  Color vertColors[3];
};
