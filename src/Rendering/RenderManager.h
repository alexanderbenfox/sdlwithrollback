#pragma once
#include "Globals.h"
#include "AssetManagement/BlitOperation.h"
#include "Core/Math/Vector2.h"
#include "Rendering/SpriteRenderer.h"
#include "Rendering/StageRenderer.h"

class Camera;

// bgfx view IDs
static constexpr uint16_t VIEW_3D_STAGE = 0;
static constexpr uint16_t VIEW_WORLD    = 1;
static constexpr uint16_t VIEW_UI       = 2;
static constexpr uint16_t VIEW_IMGUI    = 3;

//! order in the rendering order
enum class RenderLayer : int
{
  World, UI, NLayers
};

template <typename Drawable = RenderCommand>
class DrawOperator
{
public:
  DrawOperator() = default;
  //! Adds a new blit op to the list. Only objects registered here will be drawn
  void RegisterOp();
  //!
  void DeregisterOp();
  //! Used by drawn objects to pass their drawing parameters to the resource manager
  Drawable* GetAvailableOp()
  {
    if (_opIndex >= static_cast<int>(_drawableOperations.size()))
      _drawableOperations.emplace_back();
    return &_drawableOperations[_opIndex++];
  }

  //! Returns current ops for iteration
  const std::vector<Drawable>& GetOps() const { return _drawableOperations; }
  int GetOpCount() const { return _opIndex; }

  //! Resets op index for next frame
  void ResetOps() { _opIndex = 0; }

private:
  //! Index of the latest available op spot
  int _opIndex = 0;
  //! All registered blit ops
  std::vector<Drawable> _drawableOperations;

};

//______________________________________________________________________________
template <typename Drawable>
inline void DrawOperator<Drawable>::RegisterOp()
{
  _drawableOperations.push_back(Drawable());
  _drawableOperations.back().valid = false;
}

//______________________________________________________________________________
template <typename Drawable>
inline void DrawOperator<Drawable>::DeregisterOp()
{
  _drawableOperations.pop_back();
}

//______________________________________________________________________________
class RenderManager
{
public:
  //! Singleton getter
  static RenderManager& Get() { static RenderManager rm; return rm; }
  //! Inits SDL and bgfx for Metal rendering
  void Init();
  //! Destroys renderer and window
  void Destroy();
  //!
  SDL_Window* GetWindow() const { return _window; }

  //! Adds a new blit op to the list. Only objects registered here will be drawn
  template <typename Drawable>
  void RegisterDrawable(RenderLayer layer)
  {
    if constexpr (std::is_same_v<Drawable, BlitOperation<RenderType>>)
      _drawers[(int)layer].textureDrawer.RegisterOp();
    else
      _drawers[(int)layer].primitiveDrawer.RegisterOp();
  }
  //!
  template <typename Drawable>
  void DeregisterDrawable(RenderLayer layer)
  {
    if constexpr (std::is_same_v<Drawable, BlitOperation<RenderType>>)
      _drawers[(int)layer].textureDrawer.DeregisterOp();
    else
      _drawers[(int)layer].primitiveDrawer.DeregisterOp();
  }
  //! Used by drawn objects to pass their drawing parameters to the resource manager
  template <typename Drawable>
  Drawable* GetAvailableOp(RenderLayer layer)
  {
    if constexpr (std::is_same_v<Drawable, BlitOperation<RenderType>>)
      return _drawers[(int)layer].textureDrawer.GetAvailableOp();
    else
      return _drawers[(int)layer].primitiveDrawer.GetAvailableOp();
  }

  void EstablishCamera(RenderLayer layer, Camera* camera)
  {
    _drawers[(int)layer].camera = camera;
  }

  //! Preps all the sprites to be presented on screen
  void Draw();
  void Clear();
  void Present();

  Uint32 GetWindowFormat() const { return _sdlWindowFormat; }

  //! Intended just for debug drawing
  void DrawPrimitiveDebug(DrawPrimitive<RenderType>& prim, RenderLayer layer)
  {
    uint16_t viewId = (layer == RenderLayer::World) ? VIEW_WORLD : VIEW_UI;
    _spriteRenderer.SubmitRect(viewId, prim.targetRect, prim.displayColor, prim.filled);
  }

  SpriteRenderer& GetSpriteRenderer() { return _spriteRenderer; }

private:

  struct LayerDrawers
  {
    DrawOperator<BlitOperation<RenderType>> textureDrawer;
    DrawOperator<DrawPrimitive<RenderType>> primitiveDrawer;
    Camera* camera = nullptr;
  };

  LayerDrawers _drawers[(int)RenderLayer::NLayers];

  //! Window object pointer
  SDL_Window* _window;
  //!
  Uint32 _sdlWindowFormat;

  SpriteRenderer _spriteRenderer;
  StageRenderer _stageRenderer;

  //! Current bgfx backbuffer dimensions (updated on window resize via bgfx::reset)
  uint32_t _bgfxWidth = 0;
  uint32_t _bgfxHeight = 0;

  //! Initialize all sdl pointers to null and set the render scale to 1 (native size)
  RenderManager();
  RenderManager(const RenderManager&) = delete;
  RenderManager operator=(RenderManager&) = delete;

};
