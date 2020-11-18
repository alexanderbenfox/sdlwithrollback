#pragma once
#include "Core/Geometry2D/Rect.h"
#include "AssetManagement/Resource.h"
#include "Rendering/GLTexture.h"

struct DisplayImage
{
  DisplayImage() = default;
  DisplayImage(const std::string& imgPath, Rect<float> imgSubRect, int targetDisplayHeight);

  // returns window position of beginning (top left corner) of image
  Vector2<float> Show(float offsetX = 0.0f, float offsetY = 0.0f) const;

  void* ptr;
  Vector2<int> displaySize;
  Vector2<float> uv0;
  Vector2<float> uv1;

};