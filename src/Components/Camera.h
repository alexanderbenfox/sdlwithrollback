#pragma once
#include "Core/ECS/IComponent.h"
#include "Components/DebugComponent/DebugComponent.h"

#include "Core/Math/Matrix4.h"
#include "Core/Geometry2D/Rect.h"

//!
class Camera : public IDebugComponent
{
public:
  Camera() : IDebugComponent("Camera") {}
  //!
  void Init(int w, int h);
  //!
  virtual void OnDebug() override;
  //!
  DrawRect<int> rect;
  //!
  Matrix4F matrix, worldMatrix;
  //! takes care of the z value not in the transform component
  float zoom = 1.0f;

  Vector2<float> origin = Vector2<float>::Zero;

  Rect<float> clamp = Rect<float>(0, 0, m_nativeWidth, m_nativeHeight);

  Vector3<float> worldMatrixPosition;

};

//! Empty components for camera flags
struct CameraFollowsPlayers : public IComponent {};
