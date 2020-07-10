#pragma once
#include "IComponent.h"
#include "Core/Math/Matrix4.h"
#include "AssetManagement/BlitOperation.h"

//!
static bool SDLRectOverlap(const SDL_Rect& a, const SDL_Rect& b)
{
  return (b.x + b.w) >= a.x && b.x <= (a.x + a.w) &&
    (b.y + b.h) >= a.y && b.y <= (a.y + a.h);
}

//!
class Camera : public IComponent, public DebugItem
{
public:
  //!
  Camera(std::shared_ptr<Entity> entity) : IComponent(entity), DebugItem("Camera") {}
  //!
  void Init(int w, int h);
  //!
  void ConvScreenSpace(RenderCommand* entity);
  //!
  bool EntityInDisplay(const RenderCommand* entity);
  //!
  virtual void OnDebug() override
  {
    ImGui::InputFloat("Zoom level", &zoom, 0.1f, 1.0f, 2);
  }
  //!
  SDL_Rect rect;
  //!
  Matrix4F matrix;
  //! takes care of the z value not in the transform component
  float zoom = 1.0f;
  //!
  bool followPlayers = false;
  std::shared_ptr<Entity> player1, player2;
  Vector2<float> origin = Vector2<float>::Zero;

  Rect<float> clamp = Rect<float>(0, 0, m_nativeWidth, m_nativeHeight);

};

//______________________________________________________________________________
inline void Camera::ConvScreenSpace(RenderCommand* entity)
{
  entity->displayRect.x -= rect.x;
  entity->displayRect.y -= rect.y;
}

//______________________________________________________________________________
inline bool Camera::EntityInDisplay(const RenderCommand* entity)
{
  return SDLRectOverlap(rect, entity->displayRect);
}
