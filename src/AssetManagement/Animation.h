#pragma once
#include "Globals.h"

#include "AssetManagement/IAnimation.h"
#include "AssetManagement/EditableAssets/ActionAsset.h"
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
class Animation : public IAnimation
{
public:
  Animation(const std::string& sheet, const std::string& subSheet, int startIndexOnSheet, int frames, AnchorPoint anchor, const Vector2<float>& anchorPt, bool reverse);

  EventList GenerateEvents(const std::vector<EventData>& attackInfo, FrameData frameData, const Vector2<float>& textureScalingFactor);

  // --- IAnimation interface ---
  int GetFrameCount() const override { return static_cast<int>(_animFrameToSheetFrame.size()); }
  bool PlaysReverse() const override { return playReverse; }
  Vector2<double> GetRenderScaling() const override;
  void ApplyInitialFrame(RenderComponent<RenderType>& renderer, RenderProperties& properties) const override;
  void ApplyFrame(int animFrame, RenderComponent<RenderType>& renderer, RenderProperties& properties) const override;

  // --- Spritesheet-specific methods (used by editor, event generation) ---
  DrawRect<float> GetFrameSrcRect(int animFrame) const;

  template <typename Texture>
  Resource<Texture>& GetSheetTexture() const;

  DisplayImage GetGUIDisplayImage(int displayHeight, int animFrame);

  // NEED TO REMOVE THIS ASAP
  std::vector<EventData> animationEvents;

  //! Gets index on spritesheet that corresponds to this frame of animation
  int AnimFrameToSheetIndex(int frame) const { return _startIdx + _animFrameToSheetFrame[frame]; }
  //! Gets index offset from start index on sprite sheet to this frame of animation
  int AnimFrameToIndexOffset(int frame) const { return _animFrameToSheetFrame[frame]; }

  std::pair<AnchorPoint, Vector2<float>> GetAnchorForAnimFrame(int animFrame) const;

  std::string GetSubSheet() const { return _subSheetName; }

  bool playReverse = false;

  void SetAnchorPoint(AnchorPoint pt, Vector2<float> pos) { _anchorPoint = { pt, pos }; }

protected:
  //!
  std::string _spriteSheetName;
  std::string _subSheetName;
  //!
  int _frames, _startIdx;
  //!
  std::vector<int> _animFrameToSheetFrame;

  std::pair<AnchorPoint, Vector2<float>> _anchorPoint;

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

  //! Returns the abstract animation interface (used by systems)
  IAnimation* GetAnimation(const std::string& name)
  {
    auto it = _animations.find(name);
    if (it == _animations.end())
      return nullptr;
    return it->second.get();
  }

  //! Returns concrete sprite animation (used by editor code only)
  Animation* GetSpriteAnimation(const std::string& name)
  {
    auto it = _animations.find(name);
    if (it == _animations.end())
      return nullptr;
    return static_cast<Animation*>(it->second.get());
  }

  //!
  std::shared_ptr<EventList> GetEventList(const std::string& name)
  {
    if(_events.find(name) == _events.end())
      return nullptr;
    return _events.find(name)->second;
  }

  void Clear()
  {
    _animations.clear();
    _events.clear();
  }

private:
  //! Map of animation name to animation object
  std::unordered_map<std::string, std::unique_ptr<IAnimation>> _animations;
  //! Map of frame starts for events to the event that should be triggered
  std::unordered_map<std::string, std::shared_ptr<EventList>> _events;

};
