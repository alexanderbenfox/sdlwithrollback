#pragma once
#include "Components/Sprite.h"
#include "ListenerInterfaces.h"
#include "AssetManagement/Animation.h"

class Animator : public IComponent
{
public:
  Animator(std::shared_ptr<Entity> owner);

  void RegisterAnimation(const std::string& name, const char* sheet, int rows, int columns, int startIndexOnSheet, int frames);

  // Setter function
  void Play(const std::string& name, bool isLooped, bool horizontalFlip);

  void ChangeListener(IAnimatorListener* listener) { _listener = listener; }

  Animation* GetAnimationByName(const std::string& name);

  Vector2<int> GetRenderOffset(bool flipped);

  //!
  IAnimatorListener* GetListener() { return _listener; }
  //!
  Animation& GetCurrentAnimation();

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
  std::unordered_map<std::string, Animation::Key> _animations;
  //!
  std::unordered_map<std::string, Animation::Key>::iterator _currentAnimation;
  //! Top left pixel offset of the first animation registered to this
  Vector2<int> _basisRefPx;

  //std::vector<std::function<void(Animation*)>> _onAnimCompleteCallbacks;

};
