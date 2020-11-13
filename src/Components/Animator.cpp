#include "Core/ECS/Entity.h"
#include "Managers/ResourceManager.h"
#include "Components/Animator.h"
#include "Components/Collider.h"

#include "Managers/AnimationCollectionManager.h"

#include <sstream>


Animator::Animator() :
  _listener(nullptr), playing(false), looping(false), accumulatedTime(0.0f), frame(0), reverse(false), currentAnimationName(""), IComponent()
{}

Animation* Animator::Play(const std::string& name, bool isLooped, float speed, bool forcePlay)
{
  AnimationCollection& collection = AnimationCollectionManager::Get().GetCollection(animCollectionID);

  // dont play again if we are already playing it
  if (!forcePlay && (playing && name == currentAnimationName))
  {
    return collection.GetAnimation(currentAnimationName);
  }

  auto animation = collection.GetAnimationIt(name);
  if (collection.IsValid(animation))
  {
    currentAnimationName = name;
    playing = true;

    // reset all parameters
    accumulatedTime = 0;
    frame = 0;

    looping = isLooped;
    playSpeed = speed;

    reverse = animation->second.PlayReverse();
  }
  return collection.GetAnimation(currentAnimationName);
}

void Animator::Serialize(std::ostream& os) const
{
  Serializer<bool>::Serialize(os, playing);
  Serializer<bool>::Serialize(os, looping);
  Serializer<float>::Serialize(os, accumulatedTime);
  Serializer<int>::Serialize(os, frame);
  Serializer<bool>::Serialize(os, reverse);
  Serializer<std::string>::Serialize(os, currentAnimationName);
  Serializer<float>::Serialize(os, playSpeed);
  Serializer<unsigned int>::Serialize(os, animCollectionID);
}

void Animator::Deserialize(std::istream& is)
{
  Serializer<bool>::Deserialize(is, playing);
  Serializer<bool>::Deserialize(is, looping);
  Serializer<float>::Deserialize(is, accumulatedTime);
  Serializer<int>::Deserialize(is, frame);
  Serializer<bool>::Deserialize(is, reverse);
  Serializer<std::string>::Deserialize(is, currentAnimationName);
  Serializer<float>::Deserialize(is, playSpeed);
  Serializer<unsigned int>::Deserialize(is, animCollectionID);
}

//______________________________________________________________________________
std::string Animator::Log()
{
  std::stringstream ss;
  ss << "Animator: \n";
  ss << "\tIs Playing: " << playing << "\n";
  ss << "\tIs Looping: " << looping << "\n";
  ss << "\tAccumulated time: " << accumulatedTime << "\n";
  ss << "\tCurrent frame: " << frame << "\n";
  ss << "\tReverse: " << reverse << "\n";
  ss << "\tAnimation Name: " << currentAnimationName << "\n";
  ss << "\tPlay Speed: " << playSpeed << "\n";
  ss << "\tAnimation Collection ID: " << animCollectionID << "\n";
  return ss.str();
}
