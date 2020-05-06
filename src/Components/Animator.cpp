#include "Entity.h"
#include "Components/Animator.h"
#include "Components/Collider.h"
#include "ResourceManager.h"

Animator::Animator(std::shared_ptr<Entity> owner) : _listener(nullptr),
  _currentAnimation(_animations.end()), playing(false), looping(false), accumulatedTime(0.0f), frame(0), currentAnimationName(""), IComponent(owner)
{
  
}

void Animator::RegisterAnimation(const std::string& name, const char* sheet, int rows, int columns, int startIndexOnSheet, int frames)
{
  if (_animations.find(name) == _animations.end())
  {
    _animations.emplace(std::make_pair(name, Animation::Key{rows, columns, frames, startIndexOnSheet, sheet}));
    if (_animations.size() == 1)
    {
      _basisRefPx = ResourceManager::Get().GetAnimation(_animations.find(name)->second).GetRefPxLocation();
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
    return &ResourceManager::Get().GetAnimation(_animations.find(name)->second);
  return nullptr;
}

Vector2<int> Animator::GetRenderOffset(bool flipped)
{
  // set offset by aligning top left non-transparent pixels of each texture
  auto offset = Vector2<int>(
    GetCurrentAnimation().GetRefPxLocation().x - _basisRefPx.x,
    GetCurrentAnimation().GetRefPxLocation().y - _basisRefPx.y);

  if(flipped)
  {
    // create the offset for flipped
    const int entityWidth = static_cast<int>(_owner->GetComponent<RectColliderD>()->rect.Width() * (1.0f / _owner->GetComponent<Transform>()->scale.x));
    const Vector2<int> frameSize = GetCurrentAnimation().GetFrameWH();
    offset = Vector2<int>(frameSize.x - entityWidth - offset.x, offset.y);
  }
  return offset;
}

Animation& Animator::GetCurrentAnimation()
{
  return ResourceManager::Get().GetAnimation(_currentAnimation->second);
}
