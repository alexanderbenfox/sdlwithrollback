#include "AssetManagement/Animation.h"
#include "Components/Hitbox.h"
#include "Components/Rigidbody.h"
#include "Components/RenderComponent.h"
#include "Managers/GameManagement.h"
#include "Managers/ResourceManager.h"
#include <math.h>
#include <fstream>

#include <json/json.h>

//______________________________________________________________________________
Animation::Animation(const std::string& sheet, const std::string& subSheet, int startIndexOnSheet, int frames, AnchorPoint anchor, const Vector2<float>& anchorPt, bool reverse) : _startIdx(startIndexOnSheet), _frames(frames),
  _spriteSheetName(sheet), _subSheetName(subSheet), _anchorPoint(std::make_pair(anchor, anchorPt)), playReverse(reverse)
{
  int gameFrames = (int)std::ceil(frames * gameFramePerAnimationFrame);
  RebuildFrameMap(gameFrames);
  _defaultFrameMap = _animFrameToSheetFrame;
}

//______________________________________________________________________________
void Animation::RebuildFrameMap(int gameFrames)
{
  _animFrameToSheetFrame.resize(gameFrames);
  for (int i = 0; i < gameFrames; i++)
  {
    _animFrameToSheetFrame[i] = static_cast<int>(std::floor((double)i * ((double)_frames / (double)gameFrames)));
  }
}

//______________________________________________________________________________
void Animation::SetPlaybackFrameCount(int totalGameFrames)
{
  if (totalGameFrames > 0)
    RebuildFrameMap(totalGameFrames);
}

//______________________________________________________________________________
void Animation::SetPlaybackFrameMap(std::vector<int> map)
{
  _animFrameToSheetFrame = std::move(map);
}

//______________________________________________________________________________
void Animation::ClearPlaybackFrameCount()
{
  _animFrameToSheetFrame = _defaultFrameMap;
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
DisplayImage Animation::GetEditorPreview(int displayHeight, int animFrame) const
{
  auto srcRect = GetFrameSrcRect(animFrame);
  return DisplayImage(ResourceManager::Get().gSpriteSheets.Get(_spriteSheetName).src,
    Rect<float>(srcRect.x, srcRect.y, srcRect.x + srcRect.w, srcRect.y + srcRect.h), displayHeight);
}

//______________________________________________________________________________
Vector2<double> Animation::GetFrameSourceSize(int animFrame) const
{
  auto srcRect = GetFrameSrcRect(animFrame);
  return Vector2<double>(srcRect.w, srcRect.h);
}

//______________________________________________________________________________
Vector2<double> Animation::GetRenderScaling() const
{
  const SpriteSheet& ss = ResourceManager::Get().gSpriteSheets.Get(_spriteSheetName);
  return ss.renderScalingFactor;
}

//______________________________________________________________________________
void Animation::ApplyInitialFrame(RenderComponent<RenderType>& renderer, RenderProperties& properties) const
{
  auto anchor = GetAnchorForAnimFrame(0);
  properties.anchor = anchor.first;
  properties.offset = anchor.second;
  properties.renderScaling = GetRenderScaling();
  renderer.SetRenderResource(GetSheetTexture<RenderType>());
  renderer.sourceRect = GetFrameSrcRect(0);
}

//______________________________________________________________________________
void Animation::ApplyFrame(int animFrame, RenderComponent<RenderType>& renderer, RenderProperties& properties) const
{
  renderer.SetRenderResource(GetSheetTexture<RenderType>());
  renderer.sourceRect = GetFrameSrcRect(animFrame);
  properties.offset = GetAnchorForAnimFrame(animFrame).second;
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
    _animations.emplace(animationName, std::make_unique<Animation>(animationData.sheetName, animationData.subSheetName, animationData.startIndexOnSheet, animationData.frames, animationData.anchor, animationData.GetAnchorPosition(0), animationData.reverse));
  }
}

//______________________________________________________________________________
void AnimationCollection::SetAnimationEvents(const std::string& animationName, const std::vector<EventData>& eventData, const FrameData& frameData)
{
  Animation* animation = GetSpriteAnimation(animationName);
  if (animation)
  {
    auto scaling = static_cast<Vector2<float>>(animation->GetRenderScaling());
    auto [anchorPt, anchorPos] = animation->GetAnchorForAnimFrame(0);
    auto scaledOffset = anchorPos * scaling;

    auto timeline = AnimationEventHelper::ResolveSpriteTimeline(
        eventData, frameData, animation->GetSheetFrameCount(),
        scaling, anchorPt, scaledOffset);

    _events[animationName] = std::make_shared<EventList>(
        AnimationEventHelper::BuildEventList(timeline));

    if (!timeline.visualFrameMap.empty())
      animation->SetPlaybackFrameMap(std::move(timeline.visualFrameMap));
  }
}

