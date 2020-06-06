#include "Rendering/RenderManager.h"
#include "GameManagement.h"
#include "Rendering/RenderCopy.h"

#include "Components/Camera.h"

#include <type_traits>

//! Title of the game in the window... will figure this out lateerrrrr
const char* Title = "Game Title";

//______________________________________________________________________________
template <typename TextureType>
RenderManager<TextureType>::RenderManager() :
  _renderer(nullptr),
  _window(nullptr),
  _glContext(nullptr),
  _renderScale(1.0, 1.0) {}

//______________________________________________________________________________
template <typename TextureType>
void RenderManager<TextureType>::Init()
{
  SDL_Init(SDL_INIT_EVERYTHING | SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK);
  TTF_Init();

  _window = SDL_CreateWindow(Title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    m_nativeWidth, m_nativeHeight,
    SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

  _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
  SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);

  // init the gl context and a bunch of other stuff to enable GL
  if constexpr (std::is_same_v<TextureType, GLTexture>)
  {
    _glContext = SDL_GL_CreateContext(_window);
    SDL_GL_SetSwapInterval(1);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, m_nativeWidth, m_nativeHeight, 0, 0, 16);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }
}

//______________________________________________________________________________
template <typename TextureType>
void RenderManager<TextureType>::Destroy()
{
  SDL_DestroyRenderer(_renderer);
  SDL_DestroyWindow(_window);
  SDL_GL_DeleteContext(_glContext);

  _renderer = nullptr;
  _window = nullptr;

  SDL_Quit();
  TTF_Quit();

}

//______________________________________________________________________________
template <typename TextureType>
void RenderManager<TextureType>::ProcessResizeEvent(const SDL_Event& event)
{
  // get the window event size
  Vector2<int> newWindowSize(event.window.data1, event.window.data2);

  // get new scale for renderer
  _renderScale = Vector2<double>(
    static_cast<double>(newWindowSize.x) / static_cast<double>(m_nativeWidth),
    static_cast<double>(newWindowSize.y) / static_cast<double>(m_nativeHeight));

  // set render scale
  SDL_RenderSetScale(_renderer, static_cast<float>(_renderScale.x), static_cast<float>(_renderScale.y));
}

//______________________________________________________________________________
template <typename TextureType>
void RenderManager<TextureType>::RegisterBlitOp()
{
  _registeredSprites.push_back(BlitOperation<TextureType>());
  _registeredSprites.back().valid = false;
}

//______________________________________________________________________________
template <typename TextureType>
void RenderManager<TextureType>::DeregisterBlitOp()
{
  _registeredSprites.pop_back();
}

//______________________________________________________________________________
template <typename TextureType>
void RenderManager<TextureType>::Draw()
{
  auto blit = [this](BlitOperation<TextureType>* operation)
  {
    if (!operation->valid) return;

    GameManager::Get().GetMainCamera()->ConvScreenSpace(operation);
    if (GameManager::Get().GetMainCamera()->EntityInDisplay(operation))
    {
      int w, h;
      auto srcTexture = operation->_textureResource->Get();
      float rotation = 0;

      try
      {
        if constexpr (std::is_same_v<TextureType, GLTexture>)
          GL_RenderCopyEx(srcTexture, &operation->_textureRect, &operation->_displayRect, rotation, nullptr, operation->_flip);
        else if constexpr (std::is_same_v<TextureType, SDL_Texture>)
        {
          if (SDL_QueryTexture(operation->_textureResource->Get(), NULL, NULL, &w, &h) == 0)
          {
            SDL_SetTextureColorMod(operation->_textureResource->Get(), operation->_displayColor.r, operation->_displayColor.g, operation->_displayColor.b);
            SDL_RenderCopyEx(_renderer, srcTexture,
              &operation->_textureRect, &operation->_displayRect, rotation, nullptr, operation->_flip);
          }
        }
      }
      catch (std::exception& e)
      {
        std::cout << "I guess this texture isn't valid??" << "\nCaught exception: " << e.what() << "\n";
      }
    }
  };

  // only draw sprites that have been registered for this draw cycle
  for (int i = 0; i < opIndex; i++)
  {
    blit(&_registeredSprites[i]);
  }

  //reset available ops for next draw cycle
  opIndex = 0;
}

template <> void RenderManager<SDL_Texture>::Clear()
{
  SDL_RenderClear(_renderer);
}

template <> void RenderManager<GLTexture>::Clear()
{
  // clear previous render
  glClearColor(0.0, 0.0, 0.0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  // change display color
  glColor4f(1.0, 1.0, 1.0, 1.0);
}

template <> void RenderManager<SDL_Texture>::Present()
{
  SDL_RenderPresent(_renderer);
}

template <> void RenderManager<GLTexture>::Present()
{
  SDL_GL_SwapWindow(_window);
}

template class RenderManager<SDL_Texture>;
template class RenderManager<GLTexture>;
