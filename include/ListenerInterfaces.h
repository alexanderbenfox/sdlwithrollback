#pragma once
#include <string>

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

class IAction;

class IActionListener
{
public:
  virtual void OnActionComplete(IAction*) = 0;

  virtual Entity* GetOwner() = 0;

  virtual void SetStateInfo(StanceState stance, ActionState action) = 0;
};

class IAnimatorListener
{
public:
  virtual void OnAnimationComplete(const std::string& completedAnimation) = 0;
};