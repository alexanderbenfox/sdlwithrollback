#include "AssetManagement/Animation.h"
#include "Components/Hitbox.h"
#include "Components/Rigidbody.h"
#include "Managers/GameManagement.h"
#include "Managers/ResourceManager.h"
#include <math.h>
#include <fstream>

#include <json/json.h>

//______________________________________________________________________________
Animation::Animation(const SpriteSheet& sheet, int startIndexOnSheet, int frames, AnchorPoint anchor) : _startIdx(startIndexOnSheet), _frames(frames),
  _spriteSheet(sheet), _anchorPoint(std::make_pair(anchor, Vector2<int>::Zero))
{
  // initialize animation to play each sprite sheet frame 
  int gameFrames = (int)std::ceil(frames * gameFramePerAnimationFrame);
  _animFrameToSheetFrame.resize(gameFrames);
  for (int i = 0; i < gameFrames; i++)
  {
    _animFrameToSheetFrame[i] = static_cast<int>(std::floor((double)i * ((double)frames / (double)gameFrames)));
  }

  _lMargin = AnimationAsset::FindAnchorPoint(AnchorPoint::BL, _spriteSheet, _startIdx, true).x;
  _rMargin = AnimationAsset::FindAnchorPoint(AnchorPoint::BR, _spriteSheet, _startIdx, true).x;
  _tMargin = AnimationAsset::FindAnchorPoint(AnchorPoint::TL, _spriteSheet, _startIdx, true).y;

  anchorPoints[(int)AnchorPoint::TL] = AnimationAsset::FindAnchorPoint(AnchorPoint::TL, _spriteSheet, _startIdx, true);
  anchorPoints[(int)AnchorPoint::TR] = AnimationAsset::FindAnchorPoint(AnchorPoint::TR, _spriteSheet, _startIdx, true);
  anchorPoints[(int)AnchorPoint::BL] = AnimationAsset::FindAnchorPoint(AnchorPoint::BL, _spriteSheet, _startIdx, true);
  anchorPoints[(int)AnchorPoint::BR] = AnimationAsset::FindAnchorPoint(AnchorPoint::BR, _spriteSheet, _startIdx, true);

  _anchorPoint.second = AnimationAsset::FindAnchorPoint(anchor, _spriteSheet, _startIdx, true);
}

//______________________________________________________________________________
EventList Animation::GenerateEvents(const std::vector<EventData>& attackInfo, FrameData frameData, const Vector2<float>& textureScalingFactor)
{
  animationEvents = attackInfo;
  auto scaledOffset = static_cast<Vector2<float>>(_anchorPoint.second) * textureScalingFactor;
  //auto scaledOffsetAnim = GetAttachedOffset() * textureScalingFactor;
  return AnimationEventHelper::BuildEventList(textureScalingFactor, scaledOffset, attackInfo, frameData, _frames, _animFrameToSheetFrame, _anchorPoint.first);
}

//______________________________________________________________________________
DrawRect<float> Animation::GetFrameSrcRect(int animFrame) const
{
  int frame = _animFrameToSheetFrame[animFrame];
  //if invalid frame, just return nothing
  if (frame >= _frames || frame < 0)
    return { 0, 0, 0, 0 };

  int x = (_startIdx + frame) % _spriteSheet.columns;
  int y = (_startIdx + frame) / _spriteSheet.columns;
  Vector2<float> pos(x * _spriteSheet.frameSize.x, y * _spriteSheet.frameSize.y);
  return DrawRect<float>(pos.x, pos.y, _spriteSheet.frameSize.x, _spriteSheet.frameSize.y );
}

//______________________________________________________________________________
DisplayImage Animation::GetGUIDisplayImage(int displayHeight, int animFrame)
{
  //int frame = _animFrameToSheetFrame[animFrame];
  auto srcRect = GetFrameSrcRect(animFrame);

  return DisplayImage(_spriteSheet.src, Rect<float>(srcRect.x, srcRect.y, srcRect.x + srcRect.w, srcRect.y + srcRect.h), displayHeight);
}

//______________________________________________________________________________
void AnimationCollection::RegisterAnimation(const std::string& animationName, const SpriteSheet& sheet, int startIndexOnSheet, int frames, AnchorPoint anchor)
{
  if (_animations.find(animationName) == _animations.end())
  {
    _animations.emplace(std::make_pair(animationName, Animation(sheet, startIndexOnSheet, frames, anchor)));
    if (_animations.size() == 1)
    {
      Animation& mainAnim = _animations.find(animationName)->second;
      for(int pt = 0; pt < (int)AnchorPoint::Size; pt++)
      {
        _anchorPoint[pt] = AnimationAsset::FindAnchorPoint((AnchorPoint)pt, sheet, startIndexOnSheet, _useFirstSprite);
      }
      auto size = mainAnim.GetFrameWH();
      _anchorRect = Rect<int>(0, 0, size.x, size.y);
    }
  }
}

//______________________________________________________________________________
void AnimationCollection::SetAnimationEvents(const std::string& animationName, const std::vector<EventData>& eventData, const FrameData& frameData)
{
  if (_animations.find(animationName) != _animations.end())
  {
    Animation& animation = _animations.find(animationName)->second;
    if (_events.find(animationName) == _events.end())
    {
      _events.emplace(std::make_pair(animationName, std::make_shared<EventList>(animation.GenerateEvents(eventData, frameData, textureScalingFactor))));
    }
    else
    {
      //for now just replace
      _events[animationName] = std::make_shared<EventList>(animation.GenerateEvents(eventData, frameData, textureScalingFactor));
    }
  }
}

//______________________________________________________________________________
Vector2<float> AnimationCollection::GetRenderOffset(const std::string& animationName, bool flipped) const
{
  auto animIt = _animations.find(animationName);
  if(animIt == _animations.end())
    return Vector2<int>::Zero;

  Animation const& renderedAnim = animIt->second;
  // set offset by aligning top left non-transparent pixels of each texture
  AnchorPoint location = renderedAnim.GetMainAnchor().first;
  Vector2<int> anchPosition = renderedAnim.GetMainAnchor().second;

  Vector2<float> offset = anchPosition - _anchorPoint[(int)location];

  if (flipped)
    offset.x = renderedAnim.GetFrameWH().x - offset.x;

  return offset;
}
