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
  virtual void OnDebug() override
  {
    std::string name = "Camera P" + std::to_string(_owner->GetID());
    if (ImGui::CollapsingHeader(name.c_str()))
    {
      ImGui::InputFloat("Zoom level", &zoom, 0.1f, 1.0f, 2);

      ImGui::InputFloat("pos x", &worldMatrixPosition.x, 0.1f, 1.0f, 2);
      ImGui::InputFloat("pos y", &worldMatrixPosition.y, 0.1f, 1.0f, 2);
      ImGui::InputFloat("pos z", &worldMatrixPosition.z, 0.1f, 1.0f, 2);
    }
  }
  //!
  SDL_Rect rect;
  //!
  Matrix4F matrix, worldMatrix;
  //! takes care of the z value not in the transform component
  float zoom = 1.0f;

  Vector2<float> origin = Vector2<float>::Zero;

  Rect<float> clamp = Rect<float>(0, 0, m_nativeWidth, m_nativeHeight);

  Vector3<float> worldMatrixPosition;

};

//! Empty components for camera flags
struct CameraFollowsPlayers : public IComponent
{
  CameraFollowsPlayers(std::shared_ptr<Entity> e) : IComponent(e) {}
};
