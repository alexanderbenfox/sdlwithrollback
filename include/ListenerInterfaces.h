#pragma once
#include <string>

class IAction;

class IActionListener
{
public:
  virtual void OnActionComplete(IAction*) = 0;
};

class IAnimatorListener
{
public:
  virtual void OnAnimationComplete(const std::string& completedAnimation) = 0;
};