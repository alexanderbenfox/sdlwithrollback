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
  template <typename TextureType>
  void ConvScreenSpace(BlitOperation<TextureType>* entity);
  //!
  template <typename TextureType>
  bool EntityInDisplay(const BlitOperation<TextureType>* entity);
  //!
  SDL_Rect rect;
};

//______________________________________________________________________________
template <typename TextureType>
inline void Camera::ConvScreenSpace(BlitOperation<TextureType>* entity)
{
  entity->_displayRect.x -= rect.x;
  entity->_displayRect.y -= rect.y;
}

//______________________________________________________________________________
template <typename TextureType>
inline bool Camera::EntityInDisplay(const BlitOperation<TextureType>* entity)
{
  return SDLRectOverlap(rect, entity->_displayRect);
}
