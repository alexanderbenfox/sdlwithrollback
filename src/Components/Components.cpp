#include "AssetManagement/Resource.h"

#include "Entity.h"
#include "GameManagement.h"

#include "Components/Animator.h"
#include "Components/Camera.h"
#include "Components/Collider.h"
#include "Components/GameActor.h"

#include <cassert>

//______________________________________________________________________________
void Sprite::Init(const char* sheet, bool horizontalFlip)
{
  //adds new operation to the blitting list
  _display = std::make_unique<Image>(sheet);
  _horizontalFlip = horizontalFlip;
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
  _rect.x = static_cast<int>(std::floor(_owner->GetComponent<Transform>()->position.x));
  _rect.y = static_cast<int>(std::floor(_owner->GetComponent<Transform>()->position.y));
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
  _owner->GetComponent<Transform>()->position += Vector2<float>(static_cast<float>(movement.x), static_cast<float>(movement.y));
  Update(0);
}

//______________________________________________________________________________
template <typename T>
void RectCollider<T>::Update(float dt)
{
  if (!_isStatic)
  {
    Transform& transform = *_owner->GetComponent<Transform>();
    rect = Rect<T>(Vector2<T>((T)transform.position.x, (T)transform.position.y),
      Vector2<T>(transform.position.x + rect.Width(), transform.position.y + rect.Height()));
  }
}

//______________________________________________________________________________
template <typename T>
void RectCollider<T>::Draw()
{
  SDL_SetRenderDrawColor(GameManager::Get().GetRenderer(), 255, 255, 255, SDL_ALPHA_OPAQUE);

  int xBeg = static_cast<int>(std::floor(rect.Beg().x));
  int yBeg = static_cast<int>(std::floor(rect.Beg().y));
  int xEnd = static_cast<int>(std::ceil(rect.End().x));
  int yEnd = static_cast<int>(std::ceil(rect.End().y));

  SDL_Point points[5] = 
  {
    {xBeg, yBeg},
    {xBeg, yEnd},
    {xEnd, yEnd},
    {xEnd, yBeg},
    {xBeg, yBeg}
  };

  SDL_RenderDrawLines(GameManager::Get().GetRenderer(), points, 5);
  SDL_SetRenderDrawColor(GameManager::Get().GetRenderer(), 0, 0, 0, SDL_ALPHA_OPAQUE);
}

//______________________________________________________________________________
GameActor::GameActor(std::shared_ptr<Entity> owner) : _currentAction(nullptr), _newState(true), _lastInput(InputState::NONE), IComponent(owner)
{
  BeginNewAction(new LoopedAction<StanceState::STANDING, ActionState::NONE>("Idle", owner.get()));
}

//______________________________________________________________________________
void GameActor::Update(float dt)
{
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
  IAction* nextAction = action->GetFollowUpAction();
  if (nextAction == action)
    return;
  if(nextAction)
    BeginNewAction(nextAction);
}

//______________________________________________________________________________
void GameActor::BeginNewAction(IAction* action)
{
  if (_currentAction != nullptr)
    delete _currentAction;
  
  _newState = true;
  _currentAction = action;

  _currentAction->ChangeListener(this);
  _currentAction->Enact(_owner.get());
}

//______________________________________________________________________________
void GameActor::EvaluateInputContext(InputState input, const GameContext& context)
{
  if (!_newState && input == _lastInput && context == _lastContext)
    return;

  _newState = false;

  IAction* prevAction = _currentAction;
  IAction* nextAction = _currentAction->HandleInput(input, context);

  if (nextAction && (nextAction != prevAction))
  {
    // update last context
    _lastInput = input;
    _lastContext = context;

    //OnActionComplete(prevAction);
    BeginNewAction(nextAction);
  }
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
  os << phys._vel;
  os << phys._acc;
  return os;
}

std::istream& operator>>(std::istream& is, Rigidbody& phys)
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
