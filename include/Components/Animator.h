#pragma once
#include "Components/Sprite.h"
#include "ListenerInterfaces.h"
#include "AssetManagement/Animation.h"

class Animator : public IComponent
{
public:
  Animator(std::shared_ptr<Entity> owner);

  void SetAnimations(const AnimationCollection& animations);

  // Setter function
  Animation* Play(const std::string& name, bool isLooped, bool horizontalFlip);

  void ChangeListener(IAnimatorListener* listener) { _listener = listener; }

  Animation* GetCurrentAnimation() { return _animations.GetAnimation(currentAnimationName); }

  //!
  IAnimatorListener* GetListener() { return _listener; }
  //!
  AnimationCollection& AnimationLib() {return _animations; }
  // STATE VARIABLES
  //!
  bool playing;
  //!
  bool looping;
  //!
  float accumulatedTime;
  //!
  int frame;
  //!
  std::string currentAnimationName;
  

  friend std::ostream& operator<<(std::ostream& os, const Animator& animator);
  friend std::istream& operator>>(std::istream& is, Animator& animator);

protected:
  //! Things that need to know when an animation is done
  IAnimatorListener* _listener;
  //! All animations registered to this animator
  AnimationCollection _animations;

};
