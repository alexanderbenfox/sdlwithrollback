#pragma once
#include "Globals.h"

// Forward declarations — avoid pulling in full component headers
template <typename T> class RenderComponent;
class RenderProperties;

//
// Abstract animation interface.
//
// Systems (AnimationSystem, FighterFSMSystem, etc.) operate through this
// interface rather than knowing about spritesheets, skeletal rigs, or any
// specific animation format. Concrete implementations handle format details.
//
class IAnimation
{
public:
  virtual ~IAnimation() = default;

  // Total game frames in this animation
  virtual int GetFrameCount() const = 0;

  // Whether the animation plays in reverse frame order
  virtual bool PlaysReverse() const = 0;

  // Render scaling factor (used by event generation for hitbox positioning)
  virtual Vector2<double> GetRenderScaling() const = 0;

  // Called when an animation starts playing — sets up all render state for frame 0
  // (anchor, offset, scaling, texture, source rect)
  virtual void ApplyInitialFrame(RenderComponent<RenderType>& renderer, RenderProperties& properties) const = 0;

  // Called each frame the animation advances — updates per-frame render state
  // (texture, source rect, offset)
  virtual void ApplyFrame(int animFrame, RenderComponent<RenderType>& renderer, RenderProperties& properties) const = 0;
};
