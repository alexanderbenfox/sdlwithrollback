#pragma once
#include "Core/ECS/IComponent.h"
#include "Core/Interfaces/Serializable.h"
#include "AssetManagement/IAnimation.h"

class AnimationCollection;

class Animator : public IComponent, public ISerializable
{
public:
  Animator();
  // Setter function — returns abstract animation interface
  IAnimation* Play(const std::string& name, bool isLooped, float speed = 1.0f, bool forcePlay = false);

  // STATE VARIABLES
  //! Is the animator playing an animation
  bool playing;
  //! Play the animation looped (stops at last frame if false)
  bool looping;
  //! Time playing on current frame
  float accumulatedTime;
  //! Current animated frame on playing animation
  int frame;
  //! Play frames in reverse order
  bool reverse;
  //! Playing animation
  std::string currentAnimationName;
  //! Multiplier for speed of animation
  float playSpeed = 1.0f;
  //! Animation collection asset ID
  unsigned int animCollectionID;
  //! Set to true when a non-looping animation reaches its last frame
  bool animationComplete = false;

  //! Override ISerializable functions
  void Serialize(std::ostream& os) const override;
  void Deserialize(std::istream& is) override;
  std::string Log() override;

};

template <> struct ComponentInitParams<Animator>
{
  unsigned int collectionID;
  std::string name;
  bool isLooped;
  bool horizontalFlip;
  float speed = 1.0f;
  static void Init(Animator& component, const ComponentInitParams<Animator>& params)
  {
    component.animCollectionID = params.collectionID;
    component.Play(params.name, params.isLooped, params.speed);
  }
};
