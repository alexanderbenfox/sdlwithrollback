#include "Core/ECS/Entity.h"
#include "Managers/ResourceManager.h"
#include "Components/Animator.h"
#include "Components/Collider.h"


Animator::Animator() :
  _listener(nullptr), playing(false), looping(false), accumulatedTime(0.0f), frame(0), currentAnimationName(""), IComponent()
{}

void Animator::SetAnimations(AnimationCollection* animations)
{
  _animations = animations;
}

Animation* Animator::Play(const std::string& name, bool isLooped, float speed, bool forcePlay)
{
  // dont play again if we are already playing it
  if (!forcePlay && (playing && name == currentAnimationName)) return _animations->GetAnimation(currentAnimationName);
  auto animation = _animations->GetAnimationIt(name);
  if (_animations->IsValid(animation))
  {
    currentAnimationName = name;
    _currentAnimation = animation;
    playing = true;

    // reset all parameters
    accumulatedTime = 0;
    frame = 0;

    looping = isLooped;
    playSpeed = speed;
  }
  return _animations->GetAnimation(currentAnimationName);
}

void Animator::Serialize(std::ostream& os) const
{
  Serializer<bool>::Serialize(os, playing);
  Serializer<bool>::Serialize(os, looping);
  Serializer<float>::Serialize(os, accumulatedTime);
  Serializer<int>::Serialize(os, frame);
  Serializer<std::string>::Serialize(os, currentAnimationName);
  Serializer<float>::Serialize(os, playSpeed);
}

void Animator::Deserialize(std::istream& is)
{
  Serializer<bool>::Deserialize(is, playing);
  Serializer<bool>::Deserialize(is, looping);
  Serializer<float>::Deserialize(is, accumulatedTime);
  Serializer<int>::Deserialize(is, frame);
  Serializer<std::string>::Deserialize(is, currentAnimationName);
  Serializer<float>::Deserialize(is, playSpeed);

  // do this for now while we still have stupid pointers here...
  auto animation = _animations->GetAnimationIt(currentAnimationName);
  if (_animations->IsValid(animation))
    _currentAnimation = animation;
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