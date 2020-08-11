#include "AssetManagement/Resource.h"

#include "Entity.h"
#include "GameManagement.h"
#include "ResourceManager.h"

#include "Components/StateComponent.h"
#include "Components/Animator.h"
#include "Components/Camera.h"
#include "Components/Collider.h"
#include "Components/Rigidbody.h"
#include "Components/RenderComponent.h"
#include "Components/UIComponents.h"
#include "Components/Actors/GameActor.h"
#include "Core/Math/Vector2.h"

#include <cassert>

#include "Components/ActionComponents.h"

//______________________________________________________________________________
WallPushComponent::WallPushComponent(std::shared_ptr<Entity> entity) : IComponent(entity) {}

//______________________________________________________________________________
WallPushComponent::~WallPushComponent()
{
  if (_owner->GetComponent<Rigidbody>())
    _owner->GetComponent<Rigidbody>()->_vel.x = 0;
}

//______________________________________________________________________________
UIRectangleRenderComponent::UIRectangleRenderComponent(std::shared_ptr<Entity> owner) : shownSize{ 0, 0, 0, 0 }, IComponent(owner)
{
  GRenderer.RegisterDrawable<DrawPrimitive<RenderType>>(RenderLayer::UI);

  if (auto transform = owner->GetComponent<UITransform>())
  {
    shownSize.w = transform->rect.Width();
    shownSize.h = transform->rect.Height();
  }
}

//______________________________________________________________________________
UIRectangleRenderComponent::~UIRectangleRenderComponent()
{
  GRenderer.DeregisterDrawable<DrawPrimitive<RenderType>>(RenderLayer::UI);
}

//______________________________________________________________________________
void StateComponent::MarkLoser()
{
  _owner->AddComponent<LoserComponent>();
}

//______________________________________________________________________________
void StateComponent::OnDebug()
{
  std::string name = "State P" + std::to_string(_owner->GetID());
  if (ImGui::CollapsingHeader(name.c_str()))
  {
    //ImGui::Text("Player %d State Component", _owner->GetID());
    if (hitting)
      ImGui::Text("IsHitting");
    else
      ImGui::Text("NotHitting");
    ImGui::Text("HP = %d", hp);
    ImGui::Checkbox("Invulnerable", &invulnerable);

    if(ImGui::Button("Mark Loser"))
    {
      MarkLoser();
    }
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
GameActor::GameActor(std::shared_ptr<Entity> owner) : _newState(true), _lastInput(InputState::NONE), IComponent(owner)
{
}

//______________________________________________________________________________
GameActor::~GameActor()
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

std::ostream& operator<<(std::ostream& os, const Rigidbody& phys)
{
  //os << phys._vel;
  //os << phys._acc;
  return os;
}

std::istream& operator>>(std::istream& is, Rigidbody& phys)
{
  //is >> phys._vel;
  //is >> phys._acc;
  return is;
}

std::ostream& operator<<(std::ostream& os, const Animator& animator)
{
  os << animator.playing;
  os << animator.accumulatedTime;
  os << animator.frame;
  os << animator.currentAnimationName;
  return os;
}

std::istream& operator>>(std::istream& is, Animator& animator)
{
  is >> animator.playing;
  is >> animator.accumulatedTime;
  is >> animator.frame;
  is >> animator.currentAnimationName;
  //animator._currentAnimation = _animations.GetAnimation(animator.currentAnimationName);

  return is;
}

template class RectCollider<double>;
