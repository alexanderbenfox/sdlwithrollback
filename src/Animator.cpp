#include "Entity.h"
#include "Components/Sprite.h"

#include "GameManagement.h"

Animation::Animation(const char* sheet, int rows, int columns, int startIndexOnSheet, int frames) : _rows(rows), _columns(columns), _startIdx(startIndexOnSheet), _frames(frames), Image(sheet)
{
  _frameSize = Vector2<int>(_sourceRect.w / _columns, _sourceRect.h / _rows);
  _referencePx = FindReferencePixel(sheet);
}

SDL_Rect Animation::GetFrameSrcRect(int frame)
{
  //if invalid frame, just return nothing
  if (frame >= _frames || frame < 0)
    return { 0, 0, 0, 0 };

  int x = (_startIdx + frame) % _columns;
  int y = (_startIdx + frame) / _columns;

  return { x * _frameSize.x, y * _frameSize.y, _frameSize.x, _frameSize.y };
}

void Animation::SetOp(const Transform& transform, SDL_Rect rectOnTex, Vector2<int> offset, ResourceManager::BlitOperation* op)
{
  op->_textureRect = rectOnTex;
  op->_textureResource = &ResourceManager::Get().GetTexture(_src);

  int fWidth = _sourceRect.w / _columns;
  int fHeight = _sourceRect.h / _rows;

  op->_displayRect = {
    static_cast<int>(std::floorf(transform.position.x - offset.x * transform.scale.x)), static_cast<int>(std::floorf(transform.position.y - offset.y * transform.scale.y)),
    (int)(static_cast<float>(fWidth) * transform.scale.x),
    (int)(static_cast<float>(fHeight) * transform.scale.y) };

  op->valid = true;
}

Vector2<int> Animation::FindReferencePixel(const char* sheet)
{
  //detect the lowest pixel
  Texture& t = ResourceManager::Get().GetTexture(sheet);
  // make sure you know the transparent pixels
  SDL_SetTextureBlendMode(t.Get(), SDL_BLENDMODE_BLEND);

  int pitch;
  void* pixels;
  SDL_LockTexture(t.Get(), nullptr, &pixels, &pitch);
  Uint32* upixels = (Uint32*)pixels;

  // get the value of a transparent pixel (assume the first top left is transparent)
  unsigned char* px = (unsigned char*)upixels;
  Uint32 transparent = SDL_MapRGBA(SDL_GetWindowSurface(GameManager::Get().GetWindow())->format, px[0], px[1], px[2], 0x00);

  for (int y = 0; y < _frameSize.y; y++)
  {
    for (int x = 0; x < _frameSize.x; x++)
    {
      Uint32 rgba = upixels[_sourceRect.w * y + x];
      //unsigned char alpha = ((unsigned char*)&rgba)[3];
      if (rgba != transparent)
      {
        SDL_UnlockTexture(t.Get());
        return Vector2<int>(x, y);
      }
    }
  }
  SDL_UnlockTexture(t.Get());
  return Vector2<int>(0, 0);
}

Animator::Animator(std::shared_ptr<Entity> owner) : _playing(false), _accumulatedTime(0.0f), _frame(0), _nextFrameOp([](int) { return 0; }), IComponent(owner) {}

void Animator::Init()
{
  ResourceManager::Get().RegisterBlitOp();
}

void Animator::RegisterAnimation(const std::string& name, const char* sheet, int rows, int columns, int startIndexOnSheet, int frames)
{
  if (_animations.find(name) == _animations.end())
  {
    _animations.insert(std::make_pair(name, Animation(sheet, rows, columns, startIndexOnSheet, frames)));
    if (_animations.size() == 1)
    {
      _basisRefPx = _animations.find(name)->second.GetRefPxLocation();
    }
  }
}

void Animator::OnFrameBegin()
{
  _op = ResourceManager::Get().GetAvailableOp();
}

void Animator::Update(float dt)
{
  // if playing, do advance time and update frame
  if (_playing)
  {
    _accumulatedTime += dt;
    if (_accumulatedTime >= _secPerFrame)
    {
      int framesToAdv = (int)std::floorf(_accumulatedTime / _secPerFrame);

      // get next frame off of the type of anim it is
      _frame = _nextFrameOp(framesToAdv);

      // 
      _accumulatedTime -= (framesToAdv * _secPerFrame);
    }
  }

  _currentAnimation->second.SetOp(_owner->transform, _currentAnimation->second.GetFrameSrcRect(_frame), _basisOffset, _op);
}

void Animator::Play(const std::string& name, bool isLooped)
{
  // dont play again if we are already playing it
  if (_playing && name == _currentAnimationName) return;

  if (_animations.find(name) != _animations.end())
  {
    _currentAnimationName = name;
    _currentAnimation = _animations.find(name);
    _playing = true;

    // reset all parameters
    _accumulatedTime = 0;
    _frame = 0;

    // set offset by aligning top left non-transparent pixels of each texture
    _basisOffset = Vector2<int>(_currentAnimation->second.GetRefPxLocation().x - _basisRefPx.x,
      _currentAnimation->second.GetRefPxLocation().y - _basisRefPx.y);

    if (isLooped)
      _nextFrameOp = [this](int i) { return _loopAnimGetNextFrame(i); };
    else
      _nextFrameOp = [this](int i) { return _onceAnimGetNextFrame(i); };
  }
}

Animation* Animator::GetAnimationByName(const std::string& name)
{
  if (_animations.find(name) == _animations.end())
    return nullptr;
  return &_animations.find(name)->second;
}

const Animation* Animator::GetCurrentlyPlaying()
{
  return &_currentAnimation->second;
}
