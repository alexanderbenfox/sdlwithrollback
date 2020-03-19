#include "Entity.h"
#include "Components/Animator.h"
#include "Components/Hitbox.h"

#include <math.h>

#include "GameManagement.h"

Animation::Animation(const char* sheet, int rows, int columns, int startIndexOnSheet, int frames) : _rows(rows), _columns(columns), _startIdx(startIndexOnSheet), _frames(frames), Image(sheet)
{
  _frameSize = Vector2<int>(_sourceRect.w / _columns, _sourceRect.h / _rows);
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

  std::function<void(double,double,Transform*)> trigger;
  std::vector<std::function<void(double,double,Transform*)>> updates;

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
        if(startUpFrames != -1)
        {
          auto UpdateHitbox = [entityID, hitbox](double x, double y, Transform* trans)
          {
            GameManager::Get().GetEntityByID(entityID)->GetComponent<Hitbox>()->rect = Rect<double>(hitbox.Beg().x * trans->scale.x, hitbox.Beg().y * trans->scale.y, hitbox.End().x * trans->scale.x, hitbox.End().y * trans->scale.y);
            GameManager::Get().GetEntityByID(entityID)->GetComponent<Hitbox>()->rect.MoveAbsolute(Vector2<double>(x + (hitbox.Beg().x * trans->scale.x), y + (hitbox.Beg().y * trans->scale.y)));
          };
          updates.push_back(UpdateHitbox);

          if(activeFrames == -1)
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
      if(activeFrames != -1)
        recoveryFrames = i - startUpFrames + activeFrames - 1;
    }
    // construct the animated event in place
    if(startUpFrames > 0)
      _events.emplace(std::piecewise_construct, std::make_tuple(startUpFrames), std::make_tuple(startUpFrames, activeFrames, trigger, updates, DespawnHitbox));
  }
}

SDL_Rect Animation::GetFrameSrcRect(int frame)
{
  //if invalid frame, just return nothing
  if (frame >= _frames || frame < 0)
    return { 0, 0, 0, 0 };

  int x = (_startIdx + frame) % _columns;
  int y = (_startIdx + frame) / _columns;

  return OpSysConv::CreateSDLRect(x * _frameSize.x, y * _frameSize.y, _frameSize.x, _frameSize.y );
}

void Animation::SetOp(const Transform& transform, SDL_Rect rectOnTex, Vector2<int> offset, bool flip, ResourceManager::BlitOperation* op)
{
  op->_textureRect = rectOnTex;
  op->_textureResource = &_texture;

  int fWidth = _sourceRect.w / _columns;
  int fHeight = _sourceRect.h / _rows;

  op->_displayRect = OpSysConv::CreateSDLRect(
    static_cast<int>(std::floor(transform.position.x - offset.x * transform.scale.x)),
    static_cast<int>(std::floor(transform.position.y - offset.y * transform.scale.y)),
    (int)(static_cast<float>(fWidth) * transform.scale.x),
    (int)(static_cast<float>(fHeight) * transform.scale.y));

  op->_flip = flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

  op->valid = true;
}

void Animation::CheckEvents(int frame, double x, double y, Transform* transform)
{
  for (int i = 0; i < _inProgressEvents.size(); i++)
  {
    AnimationEvent* evt = _inProgressEvents[i];
    if (frame == evt->GetEndFrame())
    {
      evt->EndEvent();
      _inProgressEvents.erase(_inProgressEvents.begin() + i);
      i--;
    }
    else
    {
      evt->UpdateEvent(frame, x, y, transform);
    }
  }

  auto evtIter = _events.find(frame);
  if (evtIter != _events.end())
  {
    evtIter->second.TriggerEvent(x, y, transform);
    _inProgressEvents.push_back(&evtIter->second);
  }
}

Vector2<int> Animation::FindReferencePixel(const char* sheet)
{ 
  // Get the window format
  Uint32 windowFormat = SDL_GetWindowPixelFormat(GameManager::Get().GetWindow());
  std::shared_ptr<SDL_PixelFormat> format = std::shared_ptr<SDL_PixelFormat>(SDL_AllocFormat(windowFormat), SDL_FreeFormat);

  // Get the pixel data
  Uint32* upixels;

#ifdef _WIN32
  auto textureInfo = SDLTextureInfo(_texture.Get());
  unsigned char* px = (unsigned char*)textureInfo.pixels;
  upixels = (Uint32*)textureInfo.pixels;
  Uint32 transparent = SDL_MapRGBA(format.get(), px[0], px[1], px[2], 0x00);
#else
  upixels = (Uint32*)_texture.GetInfo().pixels.get();
#endif
  for (int y = 0; y < _frameSize.y; y++)
  {
    for (int x = 0; x < _frameSize.x; x++)
    {
      Uint32 pixel = upixels[_sourceRect.w * y + x];
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

AnimationRenderer::AnimationRenderer(std::shared_ptr<Entity> owner) : 
  _currentAnimationName(""), _currentAnimation(_animations.end()), _playing(false), _looping(false), _accumulatedTime(0.0f), _frame(0), _nextFrameOp([](int) { return 0; }), SpriteRenderer(owner)
{
  
}

void AnimationRenderer::RegisterAnimation(const std::string& name, const char* sheet, int rows, int columns, int startIndexOnSheet, int frames)
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

void AnimationRenderer::Advance(float dt)
{
  // if playing, do advance time and update frame
  if (_playing)
  {
    _accumulatedTime += dt;
    if (_accumulatedTime >= _secPerFrame)
    {
      int framesToAdv = (int)std::floor(_accumulatedTime / _secPerFrame);

      // get next frame off of the type of anim it is
      _frame = _nextFrameOp(framesToAdv);

      // 
      _accumulatedTime -= (framesToAdv * _secPerFrame);

      // when the animation is complete, do the listener callback
      if(_listener)
      {
        if(!_looping && _frame == (_currentAnimation->second.GetFrameCount() - 1))
          _listener->OnAnimationComplete(_currentAnimationName);
      }
    }
  }
}

void AnimationRenderer::Play(const std::string& name, bool isLooped, bool horizontalFlip)
{
  // dont play again if we are already playing it
  if (_playing && (name == _currentAnimationName && _horizontalFlip == horizontalFlip)) return;
  if (_animations.find(name) != _animations.end())
  {
    if(_currentAnimation != _animations.end())
      _currentAnimation->second.ClearEvents();

    _currentAnimationName = name;
    _currentAnimation = _animations.find(name);
    _playing = true;

    // reset all parameters
    _accumulatedTime = 0;
    _frame = 0;

    // set offset by aligning top left non-transparent pixels of each texture
    _basisOffset = Vector2<int>(
      _currentAnimation->second.GetRefPxLocation().x - _basisRefPx.x,
      _currentAnimation->second.GetRefPxLocation().y - _basisRefPx.y);

    _horizontalFlip = horizontalFlip;
    if(_horizontalFlip)
    {
      // create the offset for flipped
      const int entityWidth = static_cast<int>(_owner->GetComponent<RectColliderD>()->rect.Width() * (1.0f / _owner->GetComponent<Transform>()->scale.x));
      const Vector2<int> frameSize = _currentAnimation->second.GetFrameWH();
      _basisOffset = Vector2<int>(frameSize.x - entityWidth - _basisOffset.x, _basisOffset.y);
    }

    _looping = isLooped;
    if (isLooped)
      _nextFrameOp = [this](int i) { return _loopAnimGetNextFrame(i); };
    else
      _nextFrameOp = [this](int i) { return _onceAnimGetNextFrame(i); };
  }
}
