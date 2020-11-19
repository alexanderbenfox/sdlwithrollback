#pragma once
#include "AssetManagement/Animation.h"

#include "Core/Interfaces/AnimatorListener.h"

class Animator : public IComponent, public ISerializable
{
public:
  Animator();
  // Setter function
  Animation* Play(const std::string& name, bool isLooped, float speed = 1.0f, bool forcePlay = false);
  //!
  void ChangeListener(IAnimatorListener* listener) { _listener = listener; }
  //!
  IAnimatorListener* GetListener() { return _listener; }
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
  
  //! Override ISerializable functions
  void Serialize(std::ostream& os) const override;
  void Deserialize(std::istream& is) override;
  std::string Log() override;

protected:
  //! Things that need to know when an animation is done
  IAnimatorListener* _listener;

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
