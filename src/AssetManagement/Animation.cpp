#include "AssetManagement/Animation.h"
#include "Components/Hitbox.h"
#include "Components/Rigidbody.h"
#include "GameManagement.h"
#include "ResourceManager.h"
#include <math.h>
#include <fstream>

#include <json/json.h>

//______________________________________________________________________________
SpriteSheet::SpriteSheet(const char* src, int rows, int columns) : src(src), rows(rows), columns(columns),
  sheetSize(0, 0), frameSize(0, 0)
{
  GenerateSheetInfo();
}

//______________________________________________________________________________
void SpriteSheet::GenerateSheetInfo()
{
  sheetSize = ResourceManager::Get().GetTextureWidthAndHeight(src);
  frameSize = Vector2<int>(sheetSize.x / columns, sheetSize.y / rows);
}

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

  _lMargin = FindAnchorPoint(AnchorPoint::BL, true).x;
  _rMargin = FindAnchorPoint(AnchorPoint::BR, true).x;
  _tMargin = FindAnchorPoint(AnchorPoint::TL, true).y;

  _anchorPoint.second = FindAnchorPoint(anchor, true);
}

//______________________________________________________________________________
EventList Animation::GenerateEvents(const std::vector<AnimationActionEventData>& attackInfo, FrameData frameData)
{
  animationEvents = attackInfo;
  return AnimationEventHelper::BuildEventList(Vector2<int>(_lMargin, _tMargin), attackInfo, frameData, _frames, _animFrameToSheetFrame);
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
Vector2<int> Animation::FindAnchorPoint(AnchorPoint anchorType, bool fromFirstFrame) const
{
  Resource<SDL_Texture>& sheetTexture = ResourceManager::Get().GetAsset<SDL_Texture>(_spriteSheet.src);
  // Get the window format
  Uint32 windowFormat = SDL_GetWindowPixelFormat(GRenderer.GetWindow());
  std::shared_ptr<SDL_PixelFormat> format = std::shared_ptr<SDL_PixelFormat>(SDL_AllocFormat(windowFormat), SDL_FreeFormat);

  // Get the pixel data
  Uint32* upixels;
#ifdef _WIN32
  unsigned char* px = sheetTexture.GetInfo().pixels.get();
  upixels = (Uint32*)px;
#else
  upixels = (Uint32*)sheetTexture.GetInfo().pixels.get();
#endif

  auto findAnchor = [this, &upixels, &sheetTexture, &format](bool reverseX, bool reverseY, int startX, int startY)
  {
#ifdef _WIN32
    Uint32 transparent = sheetTexture.GetInfo().transparent;
#endif

    for (int yValue = startY; yValue < startY + _spriteSheet.frameSize.y; yValue++)
    {
      for (int xValue = startX; xValue < startX + _spriteSheet.frameSize.x; xValue++)
      {
        int y = yValue;
        if(reverseY)
          y = startY + _spriteSheet.frameSize.y - (yValue - startY);
        int x = xValue;
        if(reverseX)
          x = startX + _spriteSheet.frameSize.x - (xValue - startX);

        Uint32 pixel = upixels[sheetTexture.GetInfo().mWidth * y + x];
#ifdef _WIN32
        if(pixel != transparent)
#else
        Uint8 r, g, b, a;
        SDL_GetRGBA(pixel, format.get(), &r, &g, &b, &a);
        if(a == 0xFF)
#endif
          return Vector2<int>(x - startX, y - startY);
      }
    }
    return Vector2<int>(0, 0);
  };

  int startX = (_startIdx % _spriteSheet.columns) * _spriteSheet.frameSize.x;
  int startY = (_startIdx / _spriteSheet.columns) * _spriteSheet.frameSize.y;
  bool reverseX = anchorType == AnchorPoint::TR || anchorType == AnchorPoint::BR;
  bool reverseY = anchorType == AnchorPoint::BR || anchorType == AnchorPoint::BL;
  if(fromFirstFrame)
    return findAnchor(reverseX, reverseY, 0, 0);
  else
    return findAnchor(reverseX, reverseY, startX, startY);
  
}

//______________________________________________________________________________
Animation::ImGuiDisplayParams Animation::GetUVCoordsForFrame(int displayHeight, int animFrame)
{
  Resource<GLTexture>& texResource = ResourceManager::Get().GetAsset<GLTexture>(_spriteSheet.src);

  int frame = _animFrameToSheetFrame[animFrame];

  //if invalid frame
  if (frame >= _frames || frame < 0)
    return ImGuiDisplayParams{ nullptr, Vector2<int>(0, 0), Vector2<float>(0, 0), Vector2<float>(0, 0) };

  int x = (_startIdx + frame) % _spriteSheet.columns;
  int y = (_startIdx + frame) / _spriteSheet.columns;

  float u0 = ((float)x * (float)_spriteSheet.frameSize.x) / (float)_spriteSheet.sheetSize.x;
  float v0 = ((float)y * (float)_spriteSheet.frameSize.y) / (float)_spriteSheet.sheetSize.y;
  float u1 = u0 + ((float)_spriteSheet.frameSize.x / (float)_spriteSheet.sheetSize.x);
  float v1 = v0 + ((float)_spriteSheet.frameSize.y / (float)_spriteSheet.sheetSize.y);

  int displayWidth = (float)_spriteSheet.frameSize.x/(float)_spriteSheet.frameSize.y * displayHeight;

  return ImGuiDisplayParams{ (void*)(intptr_t)texResource.Get()->ID(), Vector2<int>(displayWidth, displayHeight), Vector2<float>(u0, v0), Vector2<float>(u1, v1) };
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
        _anchorPoint[pt] = mainAnim.FindAnchorPoint((AnchorPoint)pt, _useFirstSprite);
      }
      auto size = mainAnim.GetFrameWH();
      _anchorRect = Rect<int>(0, 0, size.x, size.y);
    }
  }
}

//______________________________________________________________________________
void AnimationCollection::SetAnimationEvents(const std::string& animationName, const std::vector<AnimationActionEventData>& eventData, const FrameData& frameData)
{
  if (_animations.find(animationName) != _animations.end())
  {
    Animation& animation = _animations.find(animationName)->second;
    if (_events.find(animationName) == _events.end())
    {
      _events.emplace(std::make_pair(animationName, std::make_shared<EventList>(animation.GenerateEvents(eventData, frameData))));
    }
    else
    {
      //for now just replace
      _events[animationName] = std::make_shared<EventList>(animation.GenerateEvents(eventData, frameData));
    }
  }
}

//______________________________________________________________________________
Vector2<int> AnimationCollection::GetRenderOffset(const std::string& animationName, bool flipped, int transformWidth) const
{
  auto animIt = _animations.find(animationName);
  if(animIt == _animations.end())
    return Vector2<int>::Zero;

  Animation const& renderedAnim = animIt->second;
  // set offset by aligning top left non-transparent pixels of each texture
  AnchorPoint location = renderedAnim.GetMainAnchor().first;
  Vector2<int> anchPosition = renderedAnim.GetMainAnchor().second;

  Vector2<int> offset = anchPosition - _anchorPoint[(int)location];

  if(flipped)
    offset.x += (renderedAnim.GetFlipMargin() - transformWidth);

  return offset;
}
