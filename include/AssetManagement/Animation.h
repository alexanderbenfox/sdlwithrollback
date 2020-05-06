#pragma once
#include "Components/Transform.h"
#include "AssetManagement/BlitOperation.h"
#include "ComponentConst.h"

#include <functional>
#include <unordered_map>

//!
const float secPerFrame = 1.0f / 60.0f;
const float gameFramePerAnimationFrame = (1.0f / secPerFrame) / animation_fps;

class Animation;

class AnimationEvent
{
public:
  AnimationEvent(int startFrame, int duration, std::function<void(double, double, Transform*)> onTriggerCallback, std::vector<std::function<void(double,double,Transform*)>> update, std::function<void()> onEndCallback) :
    _frame(startFrame), _duration(duration), _onTrigger(onTriggerCallback), _updates(update), _onEnd(onEndCallback) {}

  void TriggerEvent(double x, double y, Transform* trans) { _onTrigger(x, y, trans); }
  void UpdateEvent(int frame, double x, double y, Transform* trans) { _updates[frame - _frame - 1](x, y, trans); }
  void EndEvent() { _onEnd(); }
  int GetEndFrame() { return _frame + _duration; }

private:
  //! Delete copy constructor because it will only be used in the animation
  AnimationEvent(const AnimationEvent&) = delete;
  AnimationEvent operator=(AnimationEvent&) = delete;
  //! Frame this event will be called on
  int _frame;
  int _duration;
  //!
  std::function<void(double, double, Transform*)> _onTrigger;
  std::vector<std::function<void(double,double,Transform*)>> _updates;
  std::function<void()> _onEnd;
};

class Animation
{
public:
  struct Key
  {
    //!
    int rows, columns, frames, startIdx;
    //!
    std::string sheetFilePath;

    bool operator==(const Key& other)
    {
      return rows == other.rows && columns == other.columns && frames == other.frames && startIdx == other.startIdx && sheetFilePath == other.sheetFilePath;
    }

    friend bool operator==(const Key& a, const Key& b)
    {
      return a.rows == b.rows && a.columns == b.columns && a.frames == b.frames && a.startIdx == b.startIdx && a.sheetFilePath == b.sheetFilePath;
    }
    
  };

  Animation() = default;
  Animation(const char* sheet, int rows, int columns, int startIndexOnSheet, int frames);

  void AddHitboxEvents(const char* hitboxesSheet, FrameData frameData, std::shared_ptr<Entity> entity);

  //! Translates anim frame to the frame on spritesheet
  SDL_Rect GetFrameSrcRect(int animFrame);

  const int GetFrameCount() { return static_cast<int>(_animFrameToSheetFrame.size()); }

  Vector2<int> const GetFrameWH() { return _frameSize; }

  Vector2<int> const GetRefPxLocation() { return _referencePx; }
  //! Checks if an event should be trigger this frame of animation and calls its callback if so
  std::unordered_map<int, AnimationEvent>& Events() { return _events; }

  Texture& GetSheetTexture();

protected:
  //! Gets first non-transparent pixel from the top left
  Vector2<int> FindReferencePixel(const char* sheet);
  //!
  int _rows, _columns, _frames, _startIdx;
  //!
  Vector2<int> _frameSize;
  //!
  Vector2<int> _referencePx;
  //! Map of frame starts for events to the event that should be triggered
  std::unordered_map<int, AnimationEvent> _events;
  //!
  std::vector<int> _animFrameToSheetFrame;
  //!
  std::string _src;
  //!
  bool _hitboxEventRegistered;

};

template <class T>
inline void hash_combine(std::size_t & seed, const T & v)
{
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <> class std::hash<Animation::Key> {
public:
  size_t operator()(const Animation::Key& key) const
  {
    std::size_t hashResult = 0;
    hash_combine(hashResult, key.sheetFilePath);
    hash_combine(hashResult, key.rows);
    hash_combine(hashResult, key.columns);
    hash_combine(hashResult, key.frames);
    hash_combine(hashResult, key.startIdx);
    return hashResult;
  }
};
