#pragma once
#include "Components/IComponent.h"
#include "Input.h"

class IAction;

class IActionListener
{
public:
  virtual void OnActionComplete(IAction*) = 0;
};

class IAction
{
public:
  virtual ~IAction() {}
  //! 
  virtual void OnUpdate(Entity* actor) = 0;
  //!
  virtual void OnNewFrame(Entity* actor) = 0;
  //! return true if the input is handled only by the action
  virtual IAction* HandleInput(InputState bttn, CollisionSide collision, Entity* actor) = 0;
  //! Some actions might have a different rate of change than others
  virtual float GetUpdateRate() = 0;
  //!
  virtual IAction* OnActionComplete(Entity* actor) = 0;
};

struct AnimationInfo
{
  // for spawning hit boxes?
  Vector2<int> hitBoxFrames;
  std::vector<Rect<int>> hitBoxes;

  // for movement?
  // which frames is there movement on
  Vector2<int> movementFrames;
  Vector2<float> velocities;
};

//all of the possible states for animation??
enum class StanceState
{
  CROUCHING, STANDING, JUMPING
};

enum class ActionState
{
  NONE, BLOCKSTUN, HITSTUN, DASHING, LIGHT, MEDIUM, HEAVY
};

template <StanceState Stance, ActionState Action>
class AnimatedAction : public IAction
{
public:
  AnimatedAction(const std::string& animation, Entity* actor);
  //!
  AnimatedAction(const std::string& animation, Entity* actor, Vector2<float> instVeclocity);
  //!
  virtual void OnUpdate(Entity* actor) override = 0;
  //!
  virtual IAction* HandleInput(InputState bttn, CollisionSide collision, Entity* actor) override = 0;
  //!
  virtual IAction* OnActionComplete(Entity* actor) override { return nullptr; }
  //!
  virtual void OnNewFrame(Entity* actor) override {}
  //!
  virtual float GetUpdateRate() override { return animation_fps; }

protected:
  //!
  std::string _animation;

};

template <StanceState Stance, ActionState Action>
class LoopedAction : public AnimatedAction<Stance, Action>
{
public:
  LoopedAction(const std::string& animation, Entity* actor) : AnimatedAction<Stance, Action>(animation, actor) {}
  //!
  LoopedAction(const std::string& animation, Entity* actor, Vector2<float> instVeclocity) : AnimatedAction<Stance, Action>(animation, actor, instVeclocity) {}
  //!
  virtual void OnUpdate(Entity* actor) override {}
  //!
  virtual IAction* HandleInput(InputState bttn, CollisionSide collision, Entity* actor) override { return nullptr; }

protected:
  //!
  std::string _animation;

};

template <StanceState Stance, ActionState Action>
class StateLockedAnimatedAction : public AnimatedAction<Stance, Action>
{
public:
  //!
  StateLockedAnimatedAction(const std::string& animation, Entity* actor);
  //!
  virtual void OnUpdate(Entity* actor) override;
  //!
  virtual IAction* OnActionComplete(Entity* actor) override { return new LoopedAction<Stance, ActionState::NONE>(Stance == StanceState::STANDING ? "Idle" : Stance == StanceState::CROUCHING ? "Crouch" : "Jumping", actor); }
  //!
  virtual IAction* HandleInput(InputState bttn, CollisionSide collision, Entity* actor) override;
  
};

template <> IAction* StateLockedAnimatedAction<StanceState::CROUCHING, ActionState::NONE>::OnActionComplete(Entity* actor);

template <> IAction* LoopedAction<StanceState::STANDING, ActionState::NONE>::HandleInput(InputState input, CollisionSide collision, Entity* actor);

template <> IAction* LoopedAction<StanceState::JUMPING, ActionState::NONE>::HandleInput(InputState input, CollisionSide collision, Entity* actor);

template <StanceState State, ActionState Action>
inline IAction* StateLockedAnimatedAction<State, Action>::HandleInput(InputState input, CollisionSide collision, Entity* actor)
{
  if (State == StanceState::JUMPING)
  {
    if (HasState(collision, CollisionSide::DOWN))
    {
      return OnActionComplete(actor);
    }
  }
  return nullptr;
}

template <> IAction* LoopedAction<StanceState::CROUCHING, ActionState::NONE>::HandleInput(InputState input, CollisionSide collision, Entity* actor);

template <> void LoopedAction<StanceState::JUMPING, ActionState::NONE>::OnUpdate(Entity* actor);

#ifdef _WIN32
template LoopedAction<StanceState::STANDING, ActionState::NONE>;
#endif