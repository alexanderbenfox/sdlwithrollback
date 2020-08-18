#include "AssetManagement/Resource.h"

#include "Core/ECS/Entity.h"
#include "Core/Math/Vector2.h"

#include "Components/StateComponent.h"
#include "Components/Animator.h"
#include "Components/Camera.h"
#include "Components/Collider.h"
#include "Components/Rigidbody.h"
#include "Components/RenderComponent.h"
#include "Components/UIComponents.h"
#include "Components/Actors/GameActor.h"

#include "Managers/GameManagement.h"
#include "Managers/ResourceManager.h"

#include <cassert>

#include "Components/ActionComponents.h"

#include "DebugGUI/GUIController.h"

//______________________________________________________________________________
WallPushComponent::WallPushComponent() : IComponent() {}

//______________________________________________________________________________
void WallPushComponent::OnRemove(const EntityID& entity)
{
  if (ComponentArray<Rigidbody>::Get().HasComponent(entity))
    ComponentArray<Rigidbody>::Get().GetComponent(entity).velocity.x = 0;
}

//______________________________________________________________________________
UIRectangleRenderComponent::UIRectangleRenderComponent() : shownSize{ 0, 0, 0, 0 }, IComponent() {}

void UIRectangleRenderComponent::OnAdd(const EntityID& entity)
{
  GRenderer.RegisterDrawable<DrawPrimitive<RenderType>>(RenderLayer::UI);
  if (ComponentArray<UITransform>::Get().HasComponent(entity))
  {
    shownSize.w = ComponentArray<UITransform>::Get().GetComponent(entity).rect.Width();
    shownSize.h = ComponentArray<UITransform>::Get().GetComponent(entity).rect.Height();
  }
}

//______________________________________________________________________________
void UIRectangleRenderComponent::OnRemove(const EntityID& entity)
{
  GRenderer.DeregisterDrawable<DrawPrimitive<RenderType>>(RenderLayer::UI);
}

//______________________________________________________________________________
void StateComponent::OnDebug()
{
  std::string name = "State P" + std::to_string(entityID);
  if (ImGui::CollapsingHeader(name.c_str()))
  {
    //ImGui::Text("Player %d State Component", _owner->GetID());
    if (hitting)
      ImGui::Text("IsHitting");
    else
      ImGui::Text("NotHitting");
    ImGui::Text("HP = %d", hp);
    ImGui::Checkbox("Invulnerable", &invulnerable);
  }
}

//______________________________________________________________________________
void Camera::Init(int w, int h)
{
  rect.x = 0;
  rect.y = 0;
  rect.w = w;
  rect.h = h;
}

//______________________________________________________________________________
void Camera::OnDebug()
{
  std::string name = "Camera P" + std::to_string(entityID);
  if (ImGui::CollapsingHeader(name.c_str()))
  {
    ImGui::InputFloat("Zoom level", &zoom, 0.1f, 1.0f, 2);

    ImGui::InputFloat("pos x", &worldMatrixPosition.x, 0.1f, 1.0f, 2);
    ImGui::InputFloat("pos y", &worldMatrixPosition.y, 0.1f, 1.0f, 2);
    ImGui::InputFloat("pos z", &worldMatrixPosition.z, 0.1f, 1.0f, 2);
  }
}

//______________________________________________________________________________
template <typename T>
void RectCollider<T>::Init(Vector2<T> beg, Vector2<T> end)
{
  rect = unscaledRect = Rect<T>(beg, end);
}

//______________________________________________________________________________
template <typename T>
void RectCollider<T>::Draw()
{
  if (!_drawDebug)
    return;

  DrawPrimitive<RenderType> draw;
  draw.displayColor = { 255, 255, 255, 255 };
  draw.filled = false;
  draw.targetRect = DrawRect<float>(rect.beg.x, rect.beg.y, rect.Width(), rect.Height());
  draw.valid = true;

  GRenderer.DrawPrimitiveDebug(draw, RenderLayer::World);
}

//______________________________________________________________________________
GameActor::GameActor() : _newState(true), _lastInput(InputState::NONE), IComponent()
{
}

std::ostream& operator<<(std::ostream& os, const GameActor& actor)
{
  // need to figure out how to get the "_currentAction" into a serializable state...
  os << (unsigned char)actor._lastInput;
  //os << (unsigned char)actor._lastContext;
  os << actor._newState;
  return os;
}

std::istream& operator>>(std::istream& is, GameActor& actor)
{
  unsigned char lastInput, lastContext;
  is >> lastInput;
  is >> lastContext;
  is >> actor._newState;

  actor._lastInput = (InputState)lastInput;
  //actor._lastContext = (GameContext)lastContext;
  return is;
}

void Rigidbody::Serialize(std::ostream& os) const
{
  os << lastCollisionSide;
  os << velocity;
  os << acceleration;
  Serializer<bool>::Serialize(os, elasticCollisions);
  Serializer<bool>::Serialize(os, ignoreDynamicColliders);
}

void Rigidbody::Deserialize(std::istream& is)
{
  is >> lastCollisionSide;
  is >> velocity;
  is >> acceleration;
  Serializer<bool>::Deserialize(is, elasticCollisions);
  Serializer<bool>::Deserialize(is, ignoreDynamicColliders);
}

template class RectCollider<double>;
