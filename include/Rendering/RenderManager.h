#pragma once
#include "AssetManagement/BlitOperation.h"
#include "Core/Math/Vector2.h"

class Camera;

const int m_nativeWidth = 720;
const int m_nativeHeight = 480;

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
  Drawable* GetAvailableOp() { return &_drawableOperations[_opIndex++]; }
  //!
  void PerformDraw(Camera* camera);
  //!
  void PerformDraw(Drawable& operation, Camera* camera);


private:
  //!
  static void DoDraw(Drawable& operation);
  //!
  static void SetupCamera(Camera* camera);
  //!
  static void UndoCamera(Camera* camera);
  //! Index of the latest available op spot
  int _opIndex = 0;
  //! All registered blit ops. Trying to use spatial loading to make drawing faster when there are a lot of object on screen
  //! texture types are sdl textures or gl textures
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
template <typename Drawable>
inline void DrawOperator<Drawable>::PerformDraw(Camera* camera)
{
  SetupCamera(camera);
  // only draw sprites that have been registered for this draw cycle
  for (int i = 0; i < _opIndex; i++)
    DoDraw(_drawableOperations[i]);
  UndoCamera(camera);

  // reset available ops for next draw cycle
  _opIndex = 0;
}

//______________________________________________________________________________
template <typename Drawable>
inline void DrawOperator<Drawable>::PerformDraw(Drawable& operation, Camera* camera)
{
  SetupCamera(camera);
  DoDraw(operation);
  UndoCamera(camera);
}

//______________________________________________________________________________
class RenderManager
{
public:
  //! Singleton getter
  static RenderManager& Get() { static RenderManager rm; return rm; }
  //! Inits SDL for GL and regular SDL rendering
  void Init();
  //! Destroys renderer and window
  void Destroy();
  //!
  void ProcessResizeEvent(const SDL_Event& event);

  //!
  SDL_Renderer* GetRenderer() const { return _renderer; }
  //!
  SDL_Window* GetWindow() const { return _window; }
  //!
  void* GetGLContext() const { return _glContext; }

  void SwitchTo2D();

  void SwitchTo3D();

  //! Adds a new blit op to the list. Only objects registered here will be drawn
  template <typename Drawable>
  void RegisterDrawable(RenderLayer layer)
  {
    if constexpr (std::is_same_v<Drawable, BlitOperation<GLTexture>>)
      _drawers[(int)layer].textureDrawer.RegisterOp();
    else
      _drawers[(int)layer].primitiveDrawer.RegisterOp();
  }
  //!
  template <typename Drawable>
  void DeregisterDrawable(RenderLayer layer)
  {
    if constexpr (std::is_same_v<Drawable, BlitOperation<GLTexture>>)
      _drawers[(int)layer].textureDrawer.DeregisterOp();
    else
      _drawers[(int)layer].primitiveDrawer.DeregisterOp();
  }
  //! Used by drawn objects to pass their drawing parameters to the resource manager
  template <typename Drawable>
  Drawable* GetAvailableOp(RenderLayer layer)
  {
    if constexpr (std::is_same_v<Drawable, BlitOperation<GLTexture>>)
      return _drawers[(int)layer].textureDrawer.GetAvailableOp();
    else
      return _drawers[(int)layer].primitiveDrawer.GetAvailableOp();
  }

  void EstablishCamera(RenderLayer layer, Camera* camera)
  {
    _drawers[(int)layer].camera = camera;
  }

  //! Preps all the sprites to be presented on screen
  //! For SDL Textures, this will use render copy from the SDL Library and custom render copy for GL Textures
  void Draw();
  void Clear();
  void Present();

  Uint32 GetWindowFormat() const { return _sdlWindowFormat; }

  void Draw3DBackground();

  //! Intended just for debug drawing and helpers SHOULD NOT BE USED BY ENTITIES
  void DrawPrimitiveDebug(DrawPrimitive<GLTexture>& prim, RenderLayer layer)
  {
    _drawers[(int)layer].primitiveDrawer.PerformDraw(prim, _drawers[(int)layer].camera);
  }

private:

  struct LayerDrawers
  {
    DrawOperator<BlitOperation<GLTexture>> textureDrawer;
    DrawOperator<DrawPrimitive<GLTexture>> primitiveDrawer;
    Camera* camera = nullptr;
  };

  LayerDrawers _drawers[(int)RenderLayer::NLayers];

  //! SDL Renderer pointer
  SDL_Renderer* _renderer;
  //! Window object pointer
  SDL_Window* _window;
  //! SDL Gl Context pointer - only used for gl texture rendering. SDL_GLContext is just an alias for void*
  void* _glContext;
  //! Rendering scale for window resize
  Vector2<double> _renderScale;
  //!
  Uint32 _sdlWindowFormat;

  //! Initialize all sdl pointers to null and set the render scale to 1 (native size)
  RenderManager();
  RenderManager(const RenderManager&) = delete;
  RenderManager operator=(RenderManager&) = delete;

};
