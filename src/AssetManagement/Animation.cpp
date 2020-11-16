#include "AssetManagement/Animation.h"
#include "Components/Hitbox.h"
#include "Components/Rigidbody.h"
#include "Managers/GameManagement.h"
#include "Managers/ResourceManager.h"
#include <math.h>
#include <fstream>

#include <json/json.h>

//______________________________________________________________________________
Animation::Animation(const std::string& sheet, int startIndexOnSheet, int frames, AnchorPoint anchor, const Vector2<float>& anchorPt, bool reverse) : _startIdx(startIndexOnSheet), _frames(frames),
  _spriteSheetName(sheet), anchorPoint(std::make_pair(anchor, anchorPt)), reverse(reverse)
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
  auto scaledOffset = static_cast<Vector2<float>>(anchorPoint.second) * textureScalingFactor;
  //auto scaledOffsetAnim = GetAttachedOffset() * textureScalingFactor;
  return AnimationEventHelper::BuildEventList(textureScalingFactor, scaledOffset, attackInfo, frameData, _frames, _animFrameToSheetFrame, anchorPoint.first);
}

//______________________________________________________________________________
DrawRect<float> Animation::GetFrameSrcRect(int animFrame) const
{
  int frame = _animFrameToSheetFrame[animFrame];
  //if invalid frame, just return nothing
  if (frame >= _frames || frame < 0)
    return { 0, 0, 0, 0 };

  const SpriteSheet& spriteSheet = AssetLibrary<SpriteSheet>::Get(_spriteSheetName);

  int x = (_startIdx + frame) % spriteSheet.columns;
  int y = (_startIdx + frame) / spriteSheet.columns;
  Vector2<float> pos(static_cast<float>(spriteSheet.offset.x + x * spriteSheet.frameSize.x), static_cast<float>(spriteSheet.offset.y + y * spriteSheet.frameSize.y));
  return DrawRect<float>(pos.x, pos.y, static_cast<float>(spriteSheet.frameSize.x), static_cast<float>(spriteSheet.frameSize.y) );
}

//______________________________________________________________________________
Vector2<int> Animation::GetFrameWH() const
{
  return AssetLibrary<SpriteSheet>::Get(_spriteSheetName).frameSize;
}

//______________________________________________________________________________
DisplayImage Animation::GetGUIDisplayImage(int displayHeight, int animFrame)
{
  //int frame = _animFrameToSheetFrame[animFrame];
  auto srcRect = GetFrameSrcRect(animFrame);

  return DisplayImage(AssetLibrary<SpriteSheet>::Get(_spriteSheetName).src, Rect<float>(srcRect.x, srcRect.y, srcRect.x + srcRect.w, srcRect.y + srcRect.h), displayHeight);
}

//______________________________________________________________________________
Vector2<double> Animation::GetRenderScaling() const
{
  return AssetLibrary<SpriteSheet>::Get(_spriteSheetName).renderScalingFactor;
}

//______________________________________________________________________________
void AnimationCollection::RegisterAnimation(const std::string& animationName, const AnimationAsset& animationData)
{
  if (_animations.find(animationName) == _animations.end())
  {
    _animations.emplace(std::make_pair(animationName, Animation(animationData.sheetName, animationData.startIndexOnSheet, animationData.frames, animationData.anchor, animationData.GetAnchorPosition(), animationData.reverse)));
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
Vector2<float> AnimationCollection::GetRenderOffset(const std::string& animationName, bool flipped) const
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
