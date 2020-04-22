#include "Entity.h"
#include "Components/Animator.h"
#include "Components/Hitbox.h"

#include <math.h>

#include "GameManagement.h"

Animation::Animation(const char* sheet, int rows, int columns, int startIndexOnSheet, int frames) : _rows(rows), _columns(columns), _startIdx(startIndexOnSheet), _frames(frames),
  _src(ResourceManager::Get().GetTexture(sheet))
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
  _referencePx = FindReferencePixel(sheet);
}

void Animation::AddHitboxEvents(const char* hitboxesSheet, FrameData frameData, std::shared_ptr<Entity> entity)
{
  int entityID = entity->GetID();
  auto DespawnHitbox = [entityID]()
  {
    GameManager::Get().GetEntityByID(entityID)->RemoveComponent<Hitbox>();
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
          auto UpdateHitbox = [entityID, hitbox](double x, double y, Transform* trans)
          {
            GameManager::Get().GetEntityByID(entityID)->GetComponent<Hitbox>()->rect = Rect<double>(hitbox.Beg().x * trans->scale.x, hitbox.Beg().y * trans->scale.y, hitbox.End().x * trans->scale.x, hitbox.End().y * trans->scale.y);
            GameManager::Get().GetEntityByID(entityID)->GetComponent<Hitbox>()->rect.MoveAbsolute(Vector2<double>(x + (hitbox.Beg().x * trans->scale.x), y + (hitbox.Beg().y * trans->scale.y)));
          };
          updates.push_back(UpdateHitbox);

          if (activeFrames == -1)
            activeFrames = i - startUpFrames + 1;
        }
        else
        {
          startUpFrames = i;
          trigger = [entityID, hitbox, frameData](double x, double y, Transform* trans)
          {
            GameManager::Get().GetEntityByID(entityID)->AddComponent<Hitbox>();
            GameManager::Get().GetEntityByID(entityID)->GetComponent<Hitbox>()->frameData = frameData;
            GameManager::Get().GetEntityByID(entityID)->GetComponent<Hitbox>()->rect = Rect<double>(hitbox.Beg().x * trans->scale.x, hitbox.Beg().y * trans->scale.y, hitbox.End().x * trans->scale.x, hitbox.End().y * trans->scale.y);
            GameManager::Get().GetEntityByID(entityID)->GetComponent<Hitbox>()->rect.MoveAbsolute(Vector2<double>(x + (hitbox.Beg().x * trans->scale.x), y + (hitbox.Beg().y * trans->scale.y)));
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
      _events.emplace(std::piecewise_construct, std::make_tuple(frameData.startUp - 1), std::make_tuple(frameData.startUp - 1, frameData.active, trigger, updates, DespawnHitbox));
    }
  }
}

SDL_Rect Animation::GetFrameSrcRect(int animFrame)
{
  int frame = _animFrameToSheetFrame[animFrame];
  //if invalid frame, just return nothing
  if (frame >= _frames || frame < 0)
    return { 0, 0, 0, 0 };

  int x = (_startIdx + frame) % _columns;
  int y = (_startIdx + frame) / _columns;

  return OpSysConv::CreateSDLRect(x * _frameSize.x, y * _frameSize.y, _frameSize.x, _frameSize.y );
}

Vector2<int> Animation::FindReferencePixel(const char* sheet)
{
  Texture& sheetTexture = ResourceManager::Get().GetTexture(sheet);
  // Get the window format
  Uint32 windowFormat = SDL_GetWindowPixelFormat(GameManager::Get().GetWindow());
  std::shared_ptr<SDL_PixelFormat> format = std::shared_ptr<SDL_PixelFormat>(SDL_AllocFormat(windowFormat), SDL_FreeFormat);

  // Get the pixel data
  Uint32* upixels;

#ifdef _WIN32
  auto textureInfo = SDLTextureInfo(sheetTexture.Get());
  unsigned char* px = (unsigned char*)textureInfo.pixels;
  upixels = (Uint32*)textureInfo.pixels;
  Uint32 transparent = SDL_MapRGBA(format.get(), px[0], px[1], px[2], 0x00);
#else
  upixels = (Uint32*)sheetTexture.GetInfo().pixels.get();
#endif
  for (int y = 0; y < _frameSize.y; y++)
  {
    for (int x = 0; x < _frameSize.x; x++)
    {
      Uint32 pixel = upixels[sheetTexture.GetInfo().mWidth * y + x];
#ifdef _WIN32
      if(pixel != transparent)
#else
      Uint8 r, g, b, a;
      SDL_GetRGBA(pixel, format.get(), &r, &g, &b, &a);
      if(a == 0xFF)
#endif
        return Vector2<int>(x, y);
    }
  }
  return Vector2<int>(0, 0);
}

Animator::Animator(std::shared_ptr<Entity> owner) : _listener(nullptr),
  _currentAnimation(_animations.end()), playing(false), looping(false), accumulatedTime(0.0f), frame(0), currentAnimationName(""), IComponent(owner)
{
  
}

void Animator::RegisterAnimation(const std::string& name, const char* sheet, int rows, int columns, int startIndexOnSheet, int frames)
{
  if (_animations.find(name) == _animations.end())
  {
    _animations.insert(std::make_pair(name, Animation(sheet, rows, columns, startIndexOnSheet, frames)));
    if (_animations.size() == 1)
    {
      _basisRefPx = _animations.find(name)->second.GetRefPxLocation();
      //_basisRefSize = _animations.find(name)->second.GetFrameWH();
    }
  }
}

void Animator::Play(const std::string& name, bool isLooped, bool horizontalFlip)
{
  // dont play again if we are already playing it
  if (playing && name == currentAnimationName) return;
  if (_animations.find(name) != _animations.end())
  {
    currentAnimationName = name;
    _currentAnimation = _animations.find(name);
    playing = true;

    // reset all parameters
    accumulatedTime = 0;
    frame = 0;

    looping = isLooped;
  }
}

Animation* Animator::GetAnimationByName(const std::string& name)
{
  if(_animations.find(name) != _animations.end())
    return &_animations.find(name)->second;
  return nullptr;
}

Vector2<int> Animator::GetRenderOffset(bool flipped)
{
  // set offset by aligning top left non-transparent pixels of each texture
  auto offset = Vector2<int>(
    _currentAnimation->second.GetRefPxLocation().x - _basisRefPx.x,
    _currentAnimation->second.GetRefPxLocation().y - _basisRefPx.y);

  if(flipped)
  {
    // create the offset for flipped
    const int entityWidth = static_cast<int>(_owner->GetComponent<RectColliderD>()->rect.Width() * (1.0f / _owner->GetComponent<Transform>()->scale.x));
    const Vector2<int> frameSize = _currentAnimation->second.GetFrameWH();
    offset = Vector2<int>(frameSize.x - entityWidth - offset.x, offset.y);
  }
  return offset;
}
