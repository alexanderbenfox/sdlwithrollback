#pragma once
#include "AssetManagement/BlitOperation.h"
#include "Geometry.h"

const int m_nativeWidth = 600;
const int m_nativeHeight = 400;

template <typename TextureType>
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
  SDL_Renderer* GetRenderer() { return _renderer; }
  //!
  SDL_Window* GetWindow() { return _window; }

  //! Adds a new blit op to the list. Only objects registered here will be drawn
  void RegisterBlitOp();
  //!
  void DeregisterBlitOp();
  //! Used by drawn objects to pass their drawing parameters to the resource manager
  BlitOperation<TextureType>* GetAvailableOp() { return &_registeredSprites[opIndex++]; }

  //! Preps all the sprites to be presented on screen
  //! For SDL Textures, this will use render copy from the SDL Library and custom render copy for GL Textures
  void Draw();
  void Clear();
  void Present();

  

private:

  //! Number of sprites that will be drawn in the scene
  int registeredSprites = 0;
  //! Index of the latest available op spot
  int opIndex = 0;
  //! All registered blit ops. Trying to use spatial loading to make drawing faster when there are a lot of object on screen
  //! texture types are sdl textures or gl textures
  std::vector<BlitOperation<TextureType>> _registeredSprites;

  //! SDL Renderer pointer
  SDL_Renderer* _renderer;
  //! Window object pointer
  SDL_Window* _window;
  //! SDL Gl Context pointer - only used for gl texture rendering. SDL_GLContext is just an alias for void*
  void* _glContext;
  //! Rendering scale for window resize
  Vector2<double> _renderScale;

  //! Initialize all sdl pointers to null and set the render scale to 1 (native size)
  RenderManager();
  RenderManager(const RenderManager&) = delete;
  RenderManager operator=(RenderManager&) = delete;

};
