#include "Entity.h"
#include "Components/Animator.h"
#include "Components/Collider.h"
#include "ResourceManager.h"

Animator::Animator(std::shared_ptr<Entity> owner) :
  _listener(nullptr), playing(false), looping(false), accumulatedTime(0.0f), frame(0), currentAnimationName(""), IComponent(owner)
{}

void Animator::SetAnimations(const AnimationCollection& animations)
{
  _animations = animations;
}

Animation* Animator::Play(const std::string& name, bool isLooped, bool horizontalFlip)
{
  // dont play again if we are already playing it
  //if (playing && name == currentAnimationName) return _animations.GetAnimation(currentAnimationName);
  auto animation = _animations.GetAnimationIt(name);
  if (_animations.IsValid(animation))
  {
    currentAnimationName = name;
    _currentAnimation = animation;
    playing = true;

    // reset all parameters
    accumulatedTime = 0;
    frame = 0;

    looping = isLooped;
  }
  return _animations.GetAnimation(currentAnimationName);
}
