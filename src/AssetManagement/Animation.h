#pragma once
#include "Globals.h"

#include "AssetManagement/EditableAssets/ActionAsset.h"
#include "AssetManagement/EditableAssets/SpriteSheet.h"
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
  Animation(const SpriteSheet& sheet, int startIndexOnSheet, int frames, AnchorPoint anchor);

  EventList GenerateEvents(const std::vector<EventData>& attackInfo, FrameData frameData, const Vector2<float>& textureScalingFactor);

  //! Translates anim frame to the frame on spritesheet
  DrawRect<float> GetFrameSrcRect(int animFrame) const;

  const int GetFrameCount() const { return static_cast<int>(_animFrameToSheetFrame.size()); }

  Vector2<int> GetFrameWH() const { return _spriteSheet.frameSize; }
  //!
  template <typename Texture>
  Resource<Texture>& GetSheetTexture() const;
  //!
  std::pair<AnchorPoint, Vector2<int>> const& GetMainAnchor() const { return _anchorPoint; }
  //!
  Vector2<int> GetAttachedOffset() const { return Vector2<int>(_lMargin, _tMargin); }

  DisplayImage GetGUIDisplayImage(int displayHeight, int animFrame);

  // NEED TO REMOVE THIS ASAP
  std::vector<EventData> animationEvents;

  int AnimFrameToSheet(int index) const { return _animFrameToSheetFrame[index]; }

  Vector2<int> anchorPoints[(const int)AnchorPoint::Size];

protected:
  //!
  SpriteSheet _spriteSheet;
  //!
  int _frames, _startIdx;
  //! stores the bottom left and top left reference pixels
  //Vector2<int> _anchorPoints[(const int)AnchorPoint::Size];
  //!
  std::vector<int> _animFrameToSheetFrame;
  
  //!
  std::pair<AnchorPoint, Vector2<int>> _anchorPoint;
  //! finding margin from the bottom right now
  int _lMargin, _rMargin, _tMargin;
};

//______________________________________________________________________________
template <typename Texture>
inline Resource<Texture>& Animation::GetSheetTexture() const
{
  return ResourceManager::Get().GetAsset<Texture>(_spriteSheet.src);
}

//______________________________________________________________________________
// this probably should just be a component
class AnimationCollection
{
public:
  AnimationCollection() = default;
  void RegisterAnimation(const std::string& animationName, const SpriteSheet& sheet, int startIndexOnSheet, int frames, AnchorPoint anchor);
  void SetAnimationEvents(const std::string& animationName, const std::vector<EventData>& eventData, const FrameData& frameData);


  Vector2<float> textureScalingFactor;

  Vector2<float> GetRenderOffset(const std::string& animationName, bool flipped) const;
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
  //!
  Vector2<int> _anchorPoint[(const int)AnchorPoint::Size];
  Rect<int> _anchorRect;

  //! use the first sprite sheet in as anchor point reference
  bool _useFirstSprite = false;

};
