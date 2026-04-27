#pragma once
#include "Globals.h"

// Forward declarations — avoid pulling in full component headers
template <typename T> class RenderComponent;
class RenderProperties;
struct DisplayImage;

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

  // --- Editor support (animation-type-agnostic preview) ---

  // Returns an ImGui-displayable preview image for the given animation frame
  virtual DisplayImage GetEditorPreview(int displayHeight, int animFrame) const = 0;

  // Returns the source pixel dimensions of a frame (used for hitbox coordinate scaling)
  virtual Vector2<double> GetFrameSourceSize(int animFrame) const = 0;

  // Maps animation frame index to a zero-based offset (for EventData indexing)
  virtual int GetFrameIndexOffset(int animFrame) const = 0;

  // Override the animation's frame count (e.g. to match action timing).
  // The animation remaps its internal frames to fit the new count.
  virtual void SetPlaybackFrameCount(int totalGameFrames) = 0;

  // Restore the animation's natural frame count (derived from its source data).
  virtual void ClearPlaybackFrameCount() = 0;
};
