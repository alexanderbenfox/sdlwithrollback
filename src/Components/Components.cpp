#include "AssetManagement/Resource.h"

#include "Entity.h"
#include "GameManagement.h"
#include "ResourceManager.h"

#include "Components/Animator.h"
#include "Components/Camera.h"
#include "Components/Collider.h"
#include "Components/GameActor.h"
#include "Components/Rigidbody.h"
#include "Components/RenderComponent.h"

#include "Rendering/RenderCopy.h"
#include "Core/Math/Vector2.h"

#include <cassert>

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

  int xBeg = static_cast<int>(std::floor(rect.beg.x));
  int yBeg = static_cast<int>(std::floor(rect.beg.y));
  int xEnd = static_cast<int>(std::ceil(rect.end.x));
  int yEnd = static_cast<int>(std::ceil(rect.end.y));

  SDL_Point points[5] = 
  {
    {xBeg, yBeg},
    {xBeg, yEnd},
    {xEnd, yEnd},
    {xEnd, yBeg},
    {xBeg, yBeg}
  };

  if constexpr (std::is_same_v<RenderType, SDL_Texture>)
  {
    SDL_SetRenderDrawColor(GRenderer.GetRenderer(), 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawLines(GRenderer.GetRenderer(), points, 5);
    SDL_SetRenderDrawColor(GRenderer.GetRenderer(), 0, 0, 0, SDL_ALPHA_OPAQUE);
  }
  else if constexpr (std::is_same_v<RenderType, GLTexture>)
  {
    GL_RenderDrawLines(points, 5);
  }
}

//______________________________________________________________________________
GameActor::GameActor(std::shared_ptr<Entity> owner) : _currentAction(nullptr), _newState(true), _lastInput(InputState::NONE), _comboCounter(0), IComponent(owner)
{
  BeginNewAction(new LoopedAction<StanceState::STANDING, ActionState::NONE>("Idle", owner.get()));

  if constexpr (std::is_same_v<RenderType, SDL_Texture>)
  {
    _counterText = GameManager::Get().CreateEntity<Transform, RenderComponent<SDL_Texture>>();
    _counterText->GetComponent<RenderComponent<SDL_Texture>>()->Init(ResourceManager::Get().GetText("Combo: 0", "fonts\\Eurostile.ttf"));
  }
  else
  {
    _counterText = GameManager::Get().CreateEntity<Transform, TextRenderer>();
    _counterText->GetComponent<TextRenderer>()->SetFont(ResourceManager::Get().GetFontWriter("fonts\\Eurostile.ttf", 36));
  }

  // offset transform
  _counterText->GetComponent<Transform>()->position = Vector2<float>(5.0f, 20.0f);
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
  /*IAction* nextAction = action->GetFollowUpAction();
  if (nextAction == action)
    return;
  if(nextAction)
    BeginNewAction(nextAction);*/
  _newState = true;
  action->SetComplete();
}

//______________________________________________________________________________
void GameActor::BeginNewAction(IAction* action)
{
  if(action && _currentAction)
  {
    if(_currentAction->GetAction() == ActionState::HITSTUN && action->GetAction() != ActionState::HITSTUN)
    {
      _comboCounter = 0;
      // remove render properties to hide the text
      std::shared_ptr<ActionTimer> endComboText = std::shared_ptr<ActionTimer>(new SimpleActionTimer(
        [this](){ _counterText->RemoveComponent<RenderProperties>(); },
        5));
      timings.push_back(endComboText);
    }
    else if (action->GetAction() == ActionState::HITSTUN)
    {
      _counterText->AddComponent<RenderProperties>();
    }
    std::string comboText = "Combo: " + std::to_string(_comboCounter);
    if constexpr (std::is_same_v<RenderType, SDL_Texture>)
    {
      _counterText->GetComponent<RenderComponent<SDL_Texture>>()->Init(ResourceManager::Get().GetText(comboText.c_str(), "fonts\\Eurostile.ttf"));
    }
    else
    {
      _counterText->GetComponent<TextRenderer>()->SetText(comboText);
    }
  }


  if (_currentAction != nullptr)
  {
    if(_currentAction->CheckInputsOnFollowUp())
      _newState = true;
    delete _currentAction;
  }
  
  _currentAction = action;

  _currentAction->ChangeListener(this);
  _currentAction->Enact(_owner.get());
}

//______________________________________________________________________________
void GameActor::EvaluateInputContext(const InputBuffer& input, const StateComponent* stateInfo, float dt)
{
  StateComponent currentState = *stateInfo;
  if (_newState || input.Latest() != _lastInput || currentState != _lastState)
  {
    _newState = false;
    _lastInput = input.Latest();
    _lastState = currentState;

    IAction* prevAction = _currentAction;
    IAction* nextAction = _currentAction->HandleInput(input, currentState);

    if (nextAction && (nextAction != prevAction))
    {
      if(nextAction->GetAction() == ActionState::HITSTUN)
        _comboCounter++;
      //OnActionComplete(prevAction);
      BeginNewAction(nextAction);
    }
  }
}

std::ostream& operator<<(std::ostream& os, const Transform& transform)
{
  os << transform.position;
  os << transform.scale;
  os << transform.rotation;
  return os;
}

std::istream& operator>>(std::istream& is, Transform& transform)
{
  is >> transform.position;
  is >> transform.rotation;
  is >> transform.scale;
  return is;
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
