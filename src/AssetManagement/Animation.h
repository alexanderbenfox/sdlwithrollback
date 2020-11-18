#pragma once
#include "Globals.h"

#include "AssetManagement/EditableAssets/ActionAsset.h"
#include "AssetManagement/EditableAssets/AssetLibrary.h"
#include "Components/Transform.h"
#include "Components/StateComponent.h"
#include "AssetManagement/BlitOperation.h"
#include "AssetManagement/AnimationEvent.h"
#include "Managers/ResourceManager.h"

#include <functional>
#include <unordered_map>

#include "DebugGUI/DisplayImage.h"

//!

const float gameFramePerAnimationFrame = (1.0f / secPerFrame) / animation_fps;

//______________________________________________________________________________
class Animation
{
public:
  Animation(const std::string& sheet, const std::string& subSheet, int startIndexOnSheet, int frames, AnchorPoint anchor, const Vector2<float>& anchorPt, bool reverse);

  EventList GenerateEvents(const std::vector<EventData>& attackInfo, FrameData frameData, const Vector2<float>& textureScalingFactor);

  //! Translates anim frame to the frame on spritesheet
  DrawRect<float> GetFrameSrcRect(int animFrame) const;

  const int GetFrameCount() const { return static_cast<int>(_animFrameToSheetFrame.size()); }
  //!
  template <typename Texture>
  Resource<Texture>& GetSheetTexture() const;
  //!
  std::pair<AnchorPoint, Vector2<float>> const& GetMainAnchor() const { return anchorPoint; }

  DisplayImage GetGUIDisplayImage(int displayHeight, int animFrame);

  // NEED TO REMOVE THIS ASAP
  std::vector<EventData> animationEvents;

  //! Gets index on spritesheet that corresponds to this frame of animation
  int AnimFrameToSheetIndex(int frame) const { return _startIdx + _animFrameToSheetFrame[frame]; }
  //! Gets index offset from start index on sprite sheet to this frame of animation
  int AnimFrameToIndexOffset(int frame) const { return _animFrameToSheetFrame[frame]; }

  //!
  std::pair<AnchorPoint, Vector2<float>> anchorPoint;

  //!
  Vector2<double> GetRenderScaling() const;

  std::string GetSubSheet() const { return _subSheetName; }

  bool playReverse = false;

protected:
  //!
  //SpriteSheet _spriteSheet;
  std::string _spriteSheetName;
  std::string _subSheetName;
  //!
  int _frames, _startIdx;
  //! stores the bottom left and top left reference pixels
  //Vector2<int> _anchorPoints[(const int)AnchorPoint::Size];
  //!
  std::vector<int> _animFrameToSheetFrame;

};

//______________________________________________________________________________
template <typename Texture>
inline Resource<Texture>& Animation::GetSheetTexture() const
{
  return ResourceManager::Get().GetAsset<Texture>(ResourceManager::Get().gSpriteSheets.Get(_spriteSheetName).src);
}

//______________________________________________________________________________
// this probably should just be a component
class AnimationCollection
{
public:
  AnimationCollection() = default;
  void RegisterAnimation(const std::string& animationName, const AnimationAsset& animationData);
  void SetAnimationEvents(const std::string& animationName, const std::vector<EventData>& eventData, const FrameData& frameData);

  //! Getters
  Animation* GetAnimation(const std::string& name)
  {
    if(_animations.find(name) == _animations.end())
      return nullptr;
    return &_animations.find(name)->second;
  }
  //!
  std::shared_ptr<EventList> GetEventList(const std::string& name)
  {
    if(_events.find(name) == _events.end())
      return nullptr;
    return _events.find(name)->second;
  }

  std::unordered_map<std::string, Animation>::iterator GetAnimationIt(const std::string& name)
  {
    return _animations.find(name);
  }

  std::unordered_map<std::string, Animation>::iterator GetEnd()
  {
    return _animations.end();
  }

  bool IsValid(const std::unordered_map<std::string, Animation>::iterator& it)
  {
    return it != _animations.end();
  }

  void Clear()
  {
    _animations.clear();
    _events.clear();
  }

private:
  //! Map of animations name to animation object
  std::unordered_map<std::string, Animation> _animations;
  //! Map of frame starts for events to the event that should be triggered
  std::unordered_map<std::string, std::shared_ptr<EventList>> _events;

};
