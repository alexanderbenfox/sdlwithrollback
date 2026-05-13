#include "Rendering/RenderManager.h"
#include "Rendering/BgfxTexture.h"
#include "Managers/GameManagement.h"

#include "Components/Camera.h"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

//! Title of the game in the window
const char* Title = "Duel Engine";

//______________________________________________________________________________
RenderManager::RenderManager() :
  _window(nullptr) {}

//______________________________________________________________________________
void RenderManager::Init()
{
  SDL_Init(SDL_INIT_EVERYTHING | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK);
  TTF_Init();

  // Create window WITHOUT OpenGL flag — bgfx manages the GPU context
  _window = SDL_CreateWindow(Title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    m_nativeWidth, m_nativeHeight,
    SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

  // Extract native window handle for bgfx
  SDL_SysWMinfo wmInfo;
  SDL_VERSION(&wmInfo.version);
  SDL_GetWindowWMInfo(_window, &wmInfo);

  // Tell bgfx to use single-threaded rendering (call renderFrame before init)
  bgfx::renderFrame();

  // Init bgfx with platform data
  bgfx::Init bgfxInit;
  bgfxInit.type = bgfx::RendererType::Metal;
  bgfxInit.resolution.width = m_nativeWidth;
  bgfxInit.resolution.height = m_nativeHeight;
  bgfxInit.resolution.reset = BGFX_RESET_VSYNC;
#if BX_PLATFORM_OSX
  bgfxInit.platformData.nwh = wmInfo.info.cocoa.window;
#elif BX_PLATFORM_WINDOWS
  bgfxInit.platformData.nwh = wmInfo.info.win.window;
#elif BX_PLATFORM_LINUX
  bgfxInit.platformData.ndt = wmInfo.info.x11.display;
  bgfxInit.platformData.nwh = (void*)(uintptr_t)wmInfo.info.x11.window;
#endif

  if (!bgfx::init(bgfxInit))
    return;

  // Set up views
  bgfx::setViewClear(VIEW_3D_STAGE, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000FF, 1.0f, 0);
  bgfx::setViewClear(VIEW_WORLD, BGFX_CLEAR_NONE);
  bgfx::setViewClear(VIEW_UI, BGFX_CLEAR_NONE);
  bgfx::setViewClear(VIEW_IMGUI, BGFX_CLEAR_NONE);

  // Track bgfx backbuffer size
  _bgfxWidth = m_nativeWidth;
  _bgfxHeight = m_nativeHeight;

  bgfx::setViewRect(VIEW_3D_STAGE, 0, 0, uint16_t(_bgfxWidth), uint16_t(_bgfxHeight));
  bgfx::setViewRect(VIEW_WORLD, 0, 0, uint16_t(_bgfxWidth), uint16_t(_bgfxHeight));
  bgfx::setViewRect(VIEW_UI, 0, 0, uint16_t(_bgfxWidth), uint16_t(_bgfxHeight));
  bgfx::setViewRect(VIEW_IMGUI, 0, 0, uint16_t(_bgfxWidth), uint16_t(_bgfxHeight));

  _sdlWindowFormat = SDL_GetWindowPixelFormat(_window);

  // Init sub-renderers
  _spriteRenderer.Init();
  _stageRenderer.Init();
}

//______________________________________________________________________________
void RenderManager::Destroy()
{
  _stageRenderer.Shutdown();
  _spriteRenderer.Shutdown();

  // Prevent late BgfxTexture destructors from calling into bgfx after shutdown
  BgfxTexture::SetBgfxShutdown();
  bgfx::shutdown();

  SDL_DestroyWindow(_window);
  _window = nullptr;

  SDL_Quit();
  TTF_Quit();
}

//______________________________________________________________________________
void RenderManager::Draw()
{
  // Handle window resize: bgfx needs reset() to resize the backbuffer
  int windowW, windowH;
  SDL_GetWindowSize(_window, &windowW, &windowH);

  if (uint32_t(windowW) != _bgfxWidth || uint32_t(windowH) != _bgfxHeight)
  {
    _bgfxWidth = uint32_t(windowW);
    _bgfxHeight = uint32_t(windowH);
    bgfx::reset(_bgfxWidth, _bgfxHeight, BGFX_RESET_VSYNC);
    bgfx::setViewClear(VIEW_3D_STAGE, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000FF, 1.0f, 0);
    bgfx::setViewClear(VIEW_WORLD, BGFX_CLEAR_NONE);
    bgfx::setViewClear(VIEW_UI, BGFX_CLEAR_NONE);
    bgfx::setViewClear(VIEW_IMGUI, BGFX_CLEAR_NONE);
  }

  bgfx::setViewRect(VIEW_3D_STAGE, 0, 0, uint16_t(_bgfxWidth), uint16_t(_bgfxHeight));
  bgfx::setViewRect(VIEW_WORLD, 0, 0, uint16_t(_bgfxWidth), uint16_t(_bgfxHeight));
  bgfx::setViewRect(VIEW_UI, 0, 0, uint16_t(_bgfxWidth), uint16_t(_bgfxHeight));
  bgfx::setViewRect(VIEW_IMGUI, 0, 0, uint16_t(_bgfxWidth), uint16_t(_bgfxHeight));

  // 3D Stage background
  Camera* worldCamera = _drawers[(int)RenderLayer::World].camera;
  _stageRenderer.Render(worldCamera, _bgfxWidth, _bgfxHeight);

  // Set orthographic projection for 2D views
  float ortho[16];
  const bgfx::Caps* caps = bgfx::getCaps();
  bx::mtxOrtho(ortho, 0.0f, float(m_nativeWidth), float(m_nativeHeight), 0.0f, 0.0f, 16.0f, 0.0f, caps->homogeneousDepth);

  // World layer (sprites with camera offset)
  {
    float view[16];
    bx::mtxIdentity(view);

    if (worldCamera)
    {
      Vector3<float> pos = Mat4::GetPosition(worldCamera->matrix);
      // Camera translation for 2D (offset rendering position)
      bx::mtxTranslate(view, pos.x, pos.y, pos.z);
    }

    bgfx::setViewTransform(VIEW_WORLD, view, ortho);

    auto& texOps = _drawers[(int)RenderLayer::World].textureDrawer;
    for (int i = 0; i < texOps.GetOpCount(); i++)
    {
      auto& op = texOps.GetOps()[i];
      if (!op.valid || !op.textureResource) continue;

      BgfxTexture* tex = op.textureResource->Get();
      _spriteRenderer.SubmitSprite(VIEW_WORLD, tex, op.srcRect, op.targetRect, op.flip, op.displayColor);
    }
    texOps.ResetOps();

    auto& primOps = _drawers[(int)RenderLayer::World].primitiveDrawer;
    for (int i = 0; i < primOps.GetOpCount(); i++)
    {
      auto& op = primOps.GetOps()[i];
      if (!op.valid) continue;
      _spriteRenderer.SubmitRect(VIEW_WORLD, op.targetRect, op.displayColor, op.filled);
    }
    primOps.ResetOps();
  }

  // UI layer (no camera offset)
  {
    float view[16];
    bx::mtxIdentity(view);

    Camera* uiCamera = _drawers[(int)RenderLayer::UI].camera;
    if (uiCamera)
    {
      Vector3<float> pos = Mat4::GetPosition(uiCamera->matrix);
      bx::mtxTranslate(view, pos.x, pos.y, pos.z);
    }

    bgfx::setViewTransform(VIEW_UI, view, ortho);

    auto& texOps = _drawers[(int)RenderLayer::UI].textureDrawer;
    for (int i = 0; i < texOps.GetOpCount(); i++)
    {
      auto& op = texOps.GetOps()[i];
      if (!op.valid || !op.textureResource) continue;

      BgfxTexture* tex = op.textureResource->Get();
      _spriteRenderer.SubmitSprite(VIEW_UI, tex, op.srcRect, op.targetRect, op.flip, op.displayColor);
    }
    texOps.ResetOps();

    auto& primOps = _drawers[(int)RenderLayer::UI].primitiveDrawer;
    for (int i = 0; i < primOps.GetOpCount(); i++)
    {
      auto& op = primOps.GetOps()[i];
      if (!op.valid) continue;
      _spriteRenderer.SubmitRect(VIEW_UI, op.targetRect, op.displayColor, op.filled);
    }
    primOps.ResetOps();
  }
}

//______________________________________________________________________________
void RenderManager::Clear()
{
  // bgfx handles clearing per-view via setViewClear (set in Init)
  // Touch all views to ensure they render even if empty
  bgfx::touch(VIEW_3D_STAGE);
  bgfx::touch(VIEW_WORLD);
  bgfx::touch(VIEW_UI);
  bgfx::touch(VIEW_IMGUI);
}

//______________________________________________________________________________
void RenderManager::Present()
{
  bgfx::frame();
}
