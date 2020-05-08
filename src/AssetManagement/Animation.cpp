#include "AssetManagement/Animation.h"
#include "Components/Hitbox.h"
#include "GameManagement.h"
#include "ResourceManager.h"
#include <math.h>

//______________________________________________________________________________
Animation::Animation(const char* sheet, int rows, int columns, int startIndexOnSheet, int frames, AnchorPoint anchor) : _rows(rows), _columns(columns), _startIdx(startIndexOnSheet), _frames(frames),
  _src(sheet), _anchorPoint(std::make_pair(anchor, Vector2<int>::Zero()))
{
  auto sheetSize = ResourceManager::Get().GetTextureWidthAndHeight(sheet);
  _frameSize = Vector2<int>(sheetSize.x / _columns, sheetSize.y / _rows);

  // initialize animation to play each sprite sheet frame 
  int gameFrames = (int)std::ceil(frames * gameFramePerAnimationFrame);
  _animFrameToSheetFrame.resize(gameFrames);
  for (int i = 0; i < gameFrames; i++)
  {
    _animFrameToSheetFrame[i] = static_cast<int>(std::floor((double)i * ((double)frames / (double)gameFrames)));
  }
  _anchorPoint.second = FindAnchorPoint(anchor, true);
}

//______________________________________________________________________________
EventInitParams Animation::CreateHitboxEvent(const char* hitboxesSheet, FrameData frameData)
{
  auto DespawnHitbox = [](Transform* trans)
  {
    trans->RemoveComponent<Hitbox>();
  };

  std::string hitBoxFile = hitboxesSheet;
#ifndef _WIN32
  auto split = StringUtils::Split(hitBoxFile, '\\');
  if (split.size() > 1)
    hitBoxFile = StringUtils::Connect(split.begin(), split.end(), '/');
#endif
  Texture hitboxes = Texture(ResourceManager::Get().GetResourcePath() + hitBoxFile);

  int startUpFrames = -1;
  int activeFrames = -1;
  int recoveryFrames = -1;

  std::function<void(double, double, Transform*)> trigger;
  std::vector<std::function<void(double, double, Transform*)>> updates;

  hitboxes.Load();
  if (hitboxes.IsLoaded())
  {
    for (int i = 0; i < _frames; i++)
    {
      int x = (_startIdx + i) % _columns;
      int y = (_startIdx + i) / _columns;

      Rect<double> hitbox = ResourceManager::FindRect(hitboxes, _frameSize, Vector2<int>(x * _frameSize.x, y * _frameSize.y));
      if (hitbox.Area() != 0)
      {
        if (startUpFrames != -1)
        {
          auto UpdateHitbox = [hitbox](double x, double y, Transform* trans)
          {
            trans->GetComponent<Hitbox>()->rect = Rect<double>(hitbox.Beg().x * trans->scale.x, hitbox.Beg().y * trans->scale.y, hitbox.End().x * trans->scale.x, hitbox.End().y * trans->scale.y);
            trans->GetComponent<Hitbox>()->rect.MoveAbsolute(Vector2<double>(x + (hitbox.Beg().x * trans->scale.x), y + (hitbox.Beg().y * trans->scale.y)));
          };
          updates.push_back(UpdateHitbox);

          if (activeFrames == -1)
            activeFrames = i - startUpFrames + 1;
        }
        else
        {
          startUpFrames = i;
          trigger = [hitbox, frameData](double x, double y, Transform* trans)
          {
            trans->AddComponent<Hitbox>();
            trans->GetComponent<Hitbox>()->frameData = frameData;
            trans->GetComponent<Hitbox>()->rect = Rect<double>(hitbox.Beg().x * trans->scale.x, hitbox.Beg().y * trans->scale.y, hitbox.End().x * trans->scale.x, hitbox.End().y * trans->scale.y);
            trans->GetComponent<Hitbox>()->rect.MoveAbsolute(Vector2<double>(x + (hitbox.Beg().x * trans->scale.x), y + (hitbox.Beg().y * trans->scale.y)));
          };
        }
      }
    }
    if (activeFrames != -1)
      recoveryFrames = _frames - (startUpFrames + activeFrames);

    
    // using data for frame counts, construct the animated event in place
    if (startUpFrames > 0)
    {
      int animLastStartUpFrameIdx = startUpFrames - 1;
      int animLastActiveFrameIdx = animLastStartUpFrameIdx + activeFrames;
      // allow for a frame of "active" to seep into recovery for hitstop effect
      int animLastRecoveryFrameIdx = animLastActiveFrameIdx + recoveryFrames - 1;

      int lastStartUpFrameIdx = frameData.startUp - 2;
      int lastActiveFrameIdx = lastStartUpFrameIdx + frameData.active;
      int lastRecoveryFrameIdx = lastActiveFrameIdx + frameData.recover;

      int totalFramesAdjusted = lastRecoveryFrameIdx + 1;

      _animFrameToSheetFrame.resize(totalFramesAdjusted);
      for (int i = 0; i < _animFrameToSheetFrame.size(); i++)
      {

        // set up the pre active frames
        if (i <= lastStartUpFrameIdx)
        {

          //_animFrameToSheetFrame[i] = std::min(animLastStartUpFrameIdx, i);
          _animFrameToSheetFrame[i] = (int)std::ceil(static_cast<double>(i) / static_cast<double>(lastStartUpFrameIdx) * static_cast<double>(animLastStartUpFrameIdx));
        }
        else if (i <= lastActiveFrameIdx)
        {
          //_animFrameToSheetFrame[i] = std::min(animLastActiveFrameIdx, i - lastStartUpFrameIdx + animLastStartUpFrameIdx);
          double idx = static_cast<double>(i - lastStartUpFrameIdx);
          _animFrameToSheetFrame[i] = (int)std::ceil(idx / (double)frameData.active * static_cast<double>(activeFrames)) + animLastStartUpFrameIdx;
        }
        else
        {
          //_animFrameToSheetFrame[i] = std::min(animLastRecoveryFrameIdx, i - lastActiveFrameIdx + animLastActiveFrameIdx);
          double idx = static_cast<double>(i - lastActiveFrameIdx);
          //_animFrameToSheetFrame[i] = std::ceil(idx / std::ceil(fData.recover * gameFramePerAnimationFrame) * static_cast<double>(recoveryFrames)) + animLastActiveFrameIdx;
          _animFrameToSheetFrame[i] = (int)std::ceil(idx / (double)frameData.recover * static_cast<double>(recoveryFrames)) + animLastActiveFrameIdx;
        }
      }

      // do not need to adjust this because events only update on new animations
      while ((int)updates.size() < (frameData.active - 1))
        updates.push_back(updates.back());
    }
  }
  return std::make_tuple(frameData.startUp - 1, frameData.active, trigger, updates, DespawnHitbox);
}

//______________________________________________________________________________
SDL_Rect Animation::GetFrameSrcRect(int animFrame) const
{
  int frame = _animFrameToSheetFrame[animFrame];
  //if invalid frame, just return nothing
  if (frame >= _frames || frame < 0)
    return { 0, 0, 0, 0 };

  int x = (_startIdx + frame) % _columns;
  int y = (_startIdx + frame) / _columns;

  return OpSysConv::CreateSDLRect(x * _frameSize.x, y * _frameSize.y, _frameSize.x, _frameSize.y );
}

//______________________________________________________________________________
Texture& Animation::GetSheetTexture() const
{
  return ResourceManager::Get().GetTexture(_src);
}

//______________________________________________________________________________
Vector2<int> Animation::FindAnchorPoint(AnchorPoint anchorType, bool fromFirstFrame) const
{
  Texture& sheetTexture = ResourceManager::Get().GetTexture(_src);
  // Get the window format
  Uint32 windowFormat = SDL_GetWindowPixelFormat(GameManager::Get().GetWindow());
  std::shared_ptr<SDL_PixelFormat> format = std::shared_ptr<SDL_PixelFormat>(SDL_AllocFormat(windowFormat), SDL_FreeFormat);

  // Get the pixel data
  Uint32* upixels;
  #ifdef _WIN32
  unsigned char* px = sheetTexture.GetInfo().pixels.get();
  upixels = (Uint32*)px;
  Uint32 transparent = sheetTexture.GetInfo().transparent;
#else
  upixels = (Uint32*)sheetTexture.GetInfo().pixels.get();
#endif

  auto findAnchor = [this, &upixels, &sheetTexture, &format](bool reverseX, bool reverseY, int startX, int startY)
  {
    for (int yValue = startY; yValue < startY + _frameSize.y; yValue++)
    {
      for (int xValue = startX; xValue < startX + _frameSize.x; xValue++)
      {
        int y = yValue;
        if(reverseY)
          y = startY + _frameSize.y - (yValue - startY);
        int x = xValue;
        if(reverseX)
          x = startX + _frameSize.x - (xValue - startX);

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

  int startX = (_startIdx % _columns) * _frameSize.x;
  int startY = (_startIdx / _columns) * _frameSize.y;
  bool reverseX = anchorType == AnchorPoint::TR || anchorType == AnchorPoint::BR;
  bool reverseY = anchorType == AnchorPoint::BR || anchorType == AnchorPoint::BL;
  if(fromFirstFrame)
    return findAnchor(reverseX, reverseY, 0, 0);
  else
    return findAnchor(reverseX, reverseY, startX, startY);
  
}

//______________________________________________________________________________
void AnimationCollection::RegisterAnimation(const std::string& name, const char* sheet, int rows, int columns, int startIndexOnSheet, int frames, AnchorPoint anchor)
{
  if (_animations.find(name) == _animations.end())
  {
    _animations.emplace(std::make_pair(name, Animation(sheet, rows, columns, startIndexOnSheet, frames, anchor)));
    if (_animations.size() == 1)
    {
      for(int pt = 0; pt < (int)AnchorPoint::Size; pt++)
        _anchorPoint[pt] = _animations.find(name)->second.FindAnchorPoint((AnchorPoint)pt, _useFirstSprite);
    }
  }
}

//______________________________________________________________________________
void AnimationCollection::AddHitboxEvents(const std::string& animationName, const char* hitboxesSheet, FrameData frameData)
{
  if (_animations.find(animationName) != _animations.end())
  {
    Animation& animation = _animations.find(animationName)->second;
    _events.emplace(std::make_pair(animationName, std::make_shared<EventList>()));
    _events[animationName]->emplace(std::piecewise_construct, std::make_tuple(frameData.startUp - 1), animation.CreateHitboxEvent(hitboxesSheet, frameData));
  }
}

//______________________________________________________________________________
Vector2<int> AnimationCollection::GetRenderOffset(const std::string& animationName, bool flipped) const
{
  auto animIt = _animations.find(animationName);
  if(animIt == _animations.end())
    return Vector2<int>::Zero();
  // set offset by aligning top left non-transparent pixels of each texture
  AnchorPoint location = animIt->second.GetMainAnchor().first;
  Vector2<int> anchPosition = animIt->second.GetMainAnchor().second;

  Vector2<int> offset = anchPosition - _anchorPoint[(int)location];

  if(flipped)
  {
  }
  return offset;
}