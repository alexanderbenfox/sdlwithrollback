#pragma once
#include <string>

//______________________________________________________________________________
class IAnimatorListener
{
public:
  virtual void OnAnimationComplete(const std::string& completedAnimation) = 0;
};
