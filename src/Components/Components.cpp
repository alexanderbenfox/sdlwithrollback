#include "AssetManagement/Resource.h"

#include "Entity.h"
#include "GameManagement.h"

#include "Components/Sprite.h"
#include "Components/Camera.h"
#include "Components/Physics.h"
#include "Components/Collider.h"
#include "Components/GameActor.h"

#include <cassert>

//______________________________________________________________________________
void Sprite::Init(const char* sheet)
{
  //adds new operation to the blitting list
  ResourceManager::Get().RegisterBlitOp();
  _display = std::make_unique<Image>(sheet);
}

//______________________________________________________________________________
void Sprite::OnFrameBegin()
{
  _op = ResourceManager::Get().GetAvailableOp();
}

//______________________________________________________________________________
void Sprite::Update(float dt)
{
  bool flipped = false;
  if(auto pd = _owner->GetComponent<PlayerData>())
  {
    flipped = pd->flipped;
  }
  _display->SetOp(_owner->transform, _display->GetRectOnSrcText(), Vector2<int>(0, 0), flipped, _op);
}

//______________________________________________________________________________
void Camera::Init(int w, int h)
{
  _rect.x = 0;
  _rect.y = 0;
  _rect.w = w;
  _rect.h = h;
}

//______________________________________________________________________________
void Camera::Update(float dt)
{
  _rect.x = static_cast<int>(std::floor(_owner->transform.position.x));
  _rect.y = static_cast<int>(std::floor(_owner->transform.position.y));

}

//______________________________________________________________________________
void Camera::ConvScreenSpace(ResourceManager::BlitOperation* entity)
{
  entity->_displayRect.x -= _rect.x;
  entity->_displayRect.y -= _rect.y;
}

//______________________________________________________________________________
bool Camera::EntityInDisplay(const ResourceManager::BlitOperation* entity)
{
  return SDLRectOverlap(_rect, entity->_displayRect);
}

//______________________________________________________________________________
template <typename T>
void RectCollider<T>::Init(Vector2<T> beg, Vector2<T> end)
{
  rect = Rect<T>(beg, end);
}

//______________________________________________________________________________
template <typename T>
void RectCollider<T>::MoveUnit(Vector2<T> movement)
{
  _owner->transform.position += Vector2<float>(static_cast<float>(movement.x), static_cast<float>(movement.y));
  Update(0);
}

//______________________________________________________________________________
template <typename T>
void RectCollider<T>::Update(float dt)
{
  if (!_isStatic)
  {
    rect = Rect<T>(Vector2<T>((T)_owner->transform.position.x, (T)_owner->transform.position.y),
      Vector2<T>(_owner->transform.position.x + rect.Width(), _owner->transform.position.y + rect.Height()));
  }
}

//______________________________________________________________________________
GameActor::GameActor(std::shared_ptr<Entity> owner) : _currentAction(nullptr), _newState(true), _lastInput(InputState::NONE), _lastCollision(CollisionSide::NONE), IComponent(owner)
{
  BeginNewAction(new LoopedAction<StanceState::STANDING, ActionState::NONE>("Idle", owner.get()));
}

//______________________________________________________________________________
void GameActor::Update(float dt)
{
  _currentAction->OnUpdate(_owner.get());
}

//______________________________________________________________________________
void GameActor::OnFrameEnd()
{
  // complete actions
  /*for (auto finished : _actionsFinished)
  {
    if (finished == _currentAction)
    {
      delete _currentAction;
      _currentAction = nullptr;
    }
  }
  _actionsFinished.clear();*/
}

//______________________________________________________________________________
void GameActor::OnActionComplete(IAction* action)
{
  //_actionsFinished.insert(action);
  _newState = true;
}

//______________________________________________________________________________
void GameActor::HandleInput(InputState input)
{
  if (auto phys = _owner->GetComponent<Physics>())
  {
    if (!_newState && input == _lastInput && phys->GetLastCollisionSides() == _lastCollision)
      return;

    _newState = false;
    auto latestSides = phys->GetLastCollisionSides();

    IAction* prevAction = _currentAction;//(*_currentActions.begin());
    IAction* nextAction = _currentAction->HandleInput(input, latestSides, _owner.get());//(*_currentActions.begin())->HandleInput(_lastInput, _lastCollision, _owner.get());

    if (nextAction && (nextAction != prevAction))
    {
      _lastInput = input;
      _lastCollision = latestSides;

      OnActionComplete(prevAction);
      BeginNewAction(nextAction);
    }
  }
}

std::ostream& operator<<(std::ostream& os, const GameActor& actor)
{
  // need to figure out how to get the "_currentAction" into a serializable state...
  os << (unsigned char)actor._lastInput;
  os << (unsigned char)actor._lastCollision;
  os << actor._newState;
  return os;
}

std::istream& operator>>(std::istream& is, GameActor& actor)
{
  unsigned char lastInput, lastCollision;
  is >> lastInput;
  is >> lastCollision;
  is >> actor._newState;

  actor._lastInput = (InputState)lastInput;
  actor._lastCollision = (CollisionSide)lastCollision;
  return is;
}

std::ostream& operator<<(std::ostream& os, const Physics& phys)
{
  os << phys._vel;
  os << phys._acc;
  return os;
}

std::istream& operator>>(std::istream& is, Physics& phys)
{
  is >> phys._vel;
  is >> phys._acc;
  return is;
}

std::ostream& operator<<(std::ostream& os, const Animator& animator)
{
  os << animator._playing;
  os << animator._accumulatedTime;
  os << animator._frame;
  os << animator._currentAnimationName;
  return os;
}

std::istream& operator>>(std::istream& is, Animator& animator)
{
  is >> animator._playing;
  is >> animator._accumulatedTime;
  is >> animator._frame;
  is >> animator._currentAnimationName;
  animator._currentAnimation = animator._animations.find(animator._currentAnimationName);

  animator._basisOffset = Vector2<int>(
    animator._currentAnimation->second.GetRefPxLocation().x - animator._basisRefPx.x,
    animator._currentAnimation->second.GetRefPxLocation().y - animator._basisRefPx.y);

  return is;
}

template class RectCollider<double>;
