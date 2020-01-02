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
  //! 
  virtual void OnUpdate(Entity* actor) = 0;
  //!
  virtual void OnNewFrame(Entity* actor) = 0;
  //! return true if the input is handled only by the action
  virtual bool HandleInput(InputState bttn, Entity* actor) = 0;
  //! Some actions might have a different rate of change than others
  virtual float GetUpdateRate() = 0;
  //!
  virtual void OnActionComplete(Entity* actor) = 0;
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

class AnimatedAction : public IAction
{
public:
  AnimatedAction(const std::string& animation, Entity* actor);

  virtual void OnUpdate(Entity* actor) override;

  virtual void OnNewFrame(Entity* actor) override {}

  virtual bool HandleInput(InputState bttn, Entity* actor) override;

  virtual float GetUpdateRate() override { return animation_fps; }

  //!
  virtual void OnActionComplete(Entity* actor) override {}

protected:
  std::string _animation;


};

class JumpCancellable : public AnimatedAction
{
public:
  JumpCancellable(const std::string& animation, Entity* actor) : AnimatedAction(animation, actor) {}

  virtual bool HandleInput(InputState bttn, Entity* actor) override;
};

class StateModifierAction : public AnimatedAction
{
public:
  //!
  StateModifierAction(const std::string& animation, Entity* actor, StanceState state) : _state(state), AnimatedAction(animation, actor) {}
  //!
  virtual void OnActionComplete(Entity* actor) override;
private:
  //!
  StanceState _state;

};
