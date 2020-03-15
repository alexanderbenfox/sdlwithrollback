#pragma once
#include "Components/IComponent.h"
#include "Components/Input.h"
#include "ListenerInterfaces.h"

//______________________________________________________________________________
//! Inteface for a context of the world that may affect character state
class GameContext
{
public:
  GameContext() {}
  ~GameContext() {}

  bool operator==(const GameContext& other) const
  {
    return collision == other.collision && onLeftSide == other.onLeftSide && movement == other.movement;
  }

  // will merge the contexts?
  GameContext operator+(const GameContext& otherContext) const
  {
    // copy this
    GameContext newContext = *this;
    newContext.hitThisFrame |= otherContext.hitThisFrame;
    newContext.hitOnLeftSide |= otherContext.hitOnLeftSide;
    newContext.frameData.damage = otherContext.frameData.damage;
    newContext.frameData.knockback = otherContext.frameData.knockback;
    newContext.frameData.onHit = otherContext.frameData.onHit;
    newContext.frameData.onBlock = otherContext.frameData.onBlock;
    return newContext;
  }

  Vector2<float> movement;
  CollisionSide collision;
  bool onLeftSide;
  bool hitThisFrame = false;
  bool hitOnLeftSide = false;
  FrameData frameData;

};

//______________________________________________________________________________
class IAction
{
public:
  virtual ~IAction() {}

  //! Begin action
  virtual void Enact(Entity* actor) = 0;
  //! return true if the input is handled only by the action
  virtual IAction* HandleInput(const InputState& rawInput, const GameContext& context) = 0;
  //!
  virtual IAction* GetFollowUpAction() = 0;

  virtual void ChangeListener(IActionListener* listener)
  {
    _listener = listener;
  }

protected:
  //!
  IActionListener* _listener;
    //!
  virtual void OnActionComplete()
  {
    if(_listener)
      _listener->OnActionComplete(this);
  }

};

//______________________________________________________________________________
//all of the possible states for animation??
enum class StanceState
{
  CROUCHING, STANDING, JUMPING
};

enum class ActionState
{
  NONE, BLOCKSTUN, HITSTUN, DASHING, LIGHT, MEDIUM, HEAVY
};

//______________________________________________________________________________
// partial specialization
template <StanceState Stance> IAction* GetAttacksFromNeutral(const InputState& rawInput, bool facingRight);

//______________________________________________________________________________
IAction* CheckHits(const InputState& rawInput, const GameContext& context);

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
class AnimatedAction : public IAction, public IAnimatorListener
{
public:
  //!
  AnimatedAction(const std::string& animation, bool facingRight) :
    _loopedAnimation(true), _animation(animation), _facingRight(facingRight), _movementType(false) {}
  //!
  AnimatedAction(const std::string& animation, bool facingRight, Vector2<float> instVelocity) :
    _loopedAnimation(true), _animation(animation), _facingRight(facingRight), _velocity(instVelocity), _movementType(true) {}
  
  //!
  virtual void Enact(Entity* actor) override;
  //!
  virtual IAction* HandleInput(const InputState& rawInput, const GameContext& context) override = 0;

  virtual IAction* GetFollowUpAction() override {return nullptr;}

  virtual void OnAnimationComplete(const std::string& completedAnimation) override
  {
    if(_animation == completedAnimation)
      OnActionComplete();
  }

protected:
  //!
  std::string _animation;
  //!
  bool _loopedAnimation;
  //!
  bool _facingRight;
  //!
  Vector2<float> _velocity;

  //!
  bool _movementType;

};

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
class LoopedAction : public AnimatedAction<Stance, Action>
{
public:
  LoopedAction(const std::string& animation, bool facingRight) :
    AnimatedAction<Stance, Action>(animation, facingRight) {}
  //!
  LoopedAction(const std::string& animation, bool facingRight, Vector2<float> instVeclocity) :
    AnimatedAction<Stance, Action>(animation, facingRight, instVeclocity) {}

  //__________________OVERRIDES________________________________

  //!
  virtual IAction* HandleInput(const InputState& rawInput, const GameContext& context) override { return nullptr; }

};

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
class TimedAction : public LoopedAction<Stance, Action>
{
public:
  TimedAction(const std::string& animation, bool facingRight, int framesInState) : _framesDuration(framesInState),
    LoopedAction<Stance, Action>(animation, facingRight) {}
  //!
  TimedAction(const std::string& animation, bool facingRight, int framesInState, Vector2<float> instVeclocity) : _framesDuration(framesInState),
    LoopedAction<Stance, Action>(animation, facingRight, instVeclocity) {}

  //__________________OVERRIDES________________________________

  //!
  virtual void Enact(Entity* actor) override;

  //!
  virtual IAction* HandleInput(const InputState& rawInput, const GameContext& context) override
  {
    //IAction* onHitAction = CheckHits(rawInput, context);
    //if (onHitAction) return onHitAction;
    return nullptr;
  }

  virtual IAction* GetFollowUpAction() override
  {
    return new LoopedAction<Stance, ActionState::NONE>
      (Stance == StanceState::STANDING ? "Idle" : Stance == StanceState::CROUCHING ? "Crouch" : "Jumping", this->_facingRight);
  }

protected:
  int _framesDuration;

};

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
class StateLockedAnimatedAction : public AnimatedAction<Stance, Action>
{
public:
  //!
  StateLockedAnimatedAction(const std::string& animation, bool facingRight);
  //!
  StateLockedAnimatedAction(const std::string& animation, bool facingRight, Vector2<float> actionMovement);
  //!
  virtual IAction* HandleInput(const InputState& rawInput, const GameContext& context) override;
  //!
  virtual IAction* GetFollowUpAction() override;
  
};

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
class GroundedStaticAttack : public StateLockedAnimatedAction<Stance, Action>
{
public:
  //!
  GroundedStaticAttack(const std::string& animation, bool facingRight) : StateLockedAnimatedAction<Stance, Action>(animation, facingRight, Vector2<float>(0, 0)) {}

};

//______________________________________________________________________________
template <> IAction* LoopedAction<StanceState::STANDING, ActionState::NONE>::HandleInput(const InputState& rawInput, const GameContext& context);

template <> IAction* LoopedAction<StanceState::JUMPING, ActionState::NONE>::HandleInput(const InputState& rawInput, const GameContext& context);

template <> IAction* LoopedAction<StanceState::CROUCHING, ActionState::NONE>::HandleInput(const InputState& rawInput, const GameContext& context);


//______________________________________________________________________________
template <> IAction* StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::NONE>::GetFollowUpAction();

template <StanceState State, ActionState Action>
inline IAction* StateLockedAnimatedAction<State, Action>::HandleInput(const InputState& rawInput, const GameContext& context)
{
  if (context.hitThisFrame)
  {
    return new TimedAction<StanceState::STANDING, ActionState::HITSTUN>("HeavyHitstun", context.onLeftSide, context.frameData.onHit, context.frameData.knockback);
  }

  if (State == StanceState::JUMPING)
  {
    if (HasState(context.collision, CollisionSide::DOWN))
    {
      return GetFollowUpAction();
    }
  }
  return nullptr;
}

#ifdef _WIN32
template LoopedAction<StanceState::STANDING, ActionState::NONE>;
#endif