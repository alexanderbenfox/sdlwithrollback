#pragma once
#include "IComponent.h"
#include "AssetManagement/BlitOperation.h"

//!
static bool SDLRectOverlap(const SDL_Rect& a, const SDL_Rect& b)
{
  return (b.x + b.w) >= a.x && b.x <= (a.x + a.w) &&
    (b.y + b.h) >= a.y && b.y <= (a.y + a.h);
}

//!
class Camera : public IComponent
{
public:
  //!
  Camera(std::shared_ptr<Entity> entity) : IComponent(entity) {}
  //!
  void Init(int w, int h);
  //!
  void ConvScreenSpace(BlitOperation* entity);
  //!
  bool EntityInDisplay(const BlitOperation* entity);
  //!
  SDL_Rect rect;
};
