#include "AssetManagement/Animation.h"
#include "Components/Hitbox.h"
#include "Components/Rigidbody.h"
#include "Managers/GameManagement.h"
#include "Managers/ResourceManager.h"
#include <math.h>
#include <fstream>

#include <json/json.h>

//______________________________________________________________________________
Animation::Animation(const std::string& sheet, const std::string& subSheet, int startIndexOnSheet, int frames, AnchorPoint anchor, const Vector2<float>& anchorPt, bool reverse) : _startIdx(startIndexOnSheet), _frames(frames),
  _spriteSheetName(sheet), _subSheetName(subSheet), _anchorPoint(std::make_pair(anchor, anchorPt)), playReverse(reverse)
{
  // initialize animation to play each sprite sheet frame 
  int gameFrames = (int)std::ceil(frames * gameFramePerAnimationFrame);
  _animFrameToSheetFrame.resize(gameFrames);
  for (int i = 0; i < gameFrames; i++)
  {
    _animFrameToSheetFrame[i] = static_cast<int>(std::floor((double)i * ((double)frames / (double)gameFrames)));
  }
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

  const SpriteSheet& spriteSheet = ResourceManager::Get().gSpriteSheets.Get(_spriteSheetName);
  return spriteSheet.GetSubSection(_subSheetName).GetFrame(_startIdx + frame);
}

//______________________________________________________________________________
DisplayImage Animation::GetGUIDisplayImage(int displayHeight, int animFrame)
{
  //int frame = _animFrameToSheetFrame[animFrame];
  auto srcRect = GetFrameSrcRect(animFrame);

  return DisplayImage(ResourceManager::Get().gSpriteSheets.Get(_spriteSheetName).src, Rect<float>(srcRect.x, srcRect.y, srcRect.x + srcRect.w, srcRect.y + srcRect.h), displayHeight);
}

//______________________________________________________________________________
Vector2<double> Animation::GetRenderScaling() const
{
  const SpriteSheet& ss = ResourceManager::Get().gSpriteSheets.Get(_spriteSheetName);
  return ss.renderScalingFactor;
}

//______________________________________________________________________________
std::pair<AnchorPoint, Vector2<float>> Animation::GetAnchorForAnimFrame(int animFrame) const
{
  int frame = _animFrameToSheetFrame[animFrame];
  if (frame == 0 || _anchorPoint.first == AnchorPoint::TL)
    return _anchorPoint;
  else
  {
    const SpriteSheet& animSpriteSheet = ResourceManager::Get().gSpriteSheets.Get(_spriteSheetName);
    const SpriteSheet::Section& ssSection = animSpriteSheet.GetSubSection(_subSheetName);
    if (ssSection.variableSizeSprites)
    {
      DrawRect<float> anchorRect = ssSection.GetFrame(_startIdx);
      DrawRect<float> targetRect = ssSection.GetFrame(_startIdx + frame);

      AnchorPoint pt = _anchorPoint.first;
      Vector2<float> anchPos = _anchorPoint.second;
      if (pt == AnchorPoint::BL || pt == AnchorPoint::BR)
        anchPos.y += (targetRect.h - anchorRect.h);
      if (pt == AnchorPoint::BR || pt == AnchorPoint::TR)
        anchPos.x += (targetRect.w - anchorRect.w);
      return { pt, anchPos };
    }
    else
    {
      return _anchorPoint;
    }
  }
}

//______________________________________________________________________________
void AnimationCollection::RegisterAnimation(const std::string& animationName, const AnimationAsset& animationData)
{
  if (_animations.find(animationName) == _animations.end())
  {
    _animations.emplace(std::make_pair(animationName, Animation(animationData.sheetName, animationData.subSheetName, animationData.startIndexOnSheet, animationData.frames, animationData.anchor, animationData.GetAnchorPosition(0), animationData.reverse)));
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
      _events.emplace(std::make_pair(animationName, std::make_shared<EventList>(animation.GenerateEvents(eventData, frameData, animation.GetRenderScaling()))));
    }
    else
    {
      //for now just replace
      _events[animationName] = std::make_shared<EventList>(animation.GenerateEvents(eventData, frameData, animation.GetRenderScaling()));
    }
  }
}

//______________________________________________________________________________
/*Vector2<float> AnimationCollection::GetRenderOffset(const std::string& animationName, bool flipped) const
{
  auto animIt = _animations.find(animationName);
  if(animIt == _animations.end())
    return Vector2<int>::Zero;

  Animation const& renderedAnim = animIt->second;
  Vector2<float> offset = renderedAnim.GetMainAnchor().second;

  if (flipped)
    offset.x = renderedAnim.GetFrameWH().x - offset.x;

  return offset;
}
*/