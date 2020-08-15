#include "AssetManagement/Resource.h"
#include "Managers/GameManagement.h"

#include <SDL2/SDL.h>

#include <iostream>

template <typename T>
void Resource<T>::Unload()
{
  _resource.reset();
  _resource = nullptr;
}

template <> void Resource<SDL_Texture>::Load()
{
  if (_loaded) return;
  
  Uint32 windowFormat = GRenderer.GetWindowFormat();

  SDL_Surface* surface = IMG_Load(_pathToResource.c_str());
  if (surface)
  {
    SDL_Surface* unformattedSurface = surface;
    surface = SDL_ConvertSurfaceFormat(surface, windowFormat, 0);
    // Set blend mode for alpha blending
    if (SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND) != 0)
    {
      std::cout << "Setting Blend Mode failed: " << SDL_GetError() << "\n";
      return;
    }
    SDL_FreeSurface(unformattedSurface);
    unformattedSurface = NULL;
  }
  if (surface)
  {
    SDL_Texture* texture = SDL_CreateTexture(GRenderer.GetRenderer(), windowFormat, SDL_TEXTUREACCESS_STREAMING, surface->w, surface->h);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    void* pixels;
     // Fill out information for the texture
    SDL_LockTexture(texture, NULL, &pixels, &_info.mPitch);
    // copy pixels from old surface to new surface
    std::memcpy(pixels, surface->pixels, surface->pitch * surface->h);

//#ifndef _WIN32
    //for some reason, i havent been able to lock/unlock textures on mac without wiping them
    _info.pixels = std::unique_ptr<unsigned char>(new unsigned char[surface->pitch * surface->h]);
    std::memcpy(_info.pixels.get(), surface->pixels, surface->pitch * surface->h);
//#endif

    std::shared_ptr<SDL_PixelFormat> format = std::shared_ptr<SDL_PixelFormat>(SDL_AllocFormat(windowFormat), SDL_FreeFormat);
    unsigned char* px = (unsigned char*)pixels;
    _info.transparent = SDL_MapRGBA(format.get(), px[0], px[1], px[2], 0x00);

    // Unlock texture so it can update
    SDL_UnlockTexture(texture);
    pixels = nullptr;

    _info.mHeight = surface->h;
    _info.mWidth = surface->w;

    _resource = std::shared_ptr<SDL_Texture>(texture, SDL_DestroyTexture);
    if (_resource) _loaded = true;

    SDL_FreeSurface(surface);
    surface = NULL;
  }
}

template <> void Resource<SDL_Surface>::Load()
{
  if (_loaded) return;

  _resource = std::shared_ptr<SDL_Surface>(IMG_Load(_pathToResource.c_str()), SDL_FreeSurface);
  if (_resource) _loaded = true;
}

template <> void Resource<TTF_Font>::Load()
{
  if (_loaded) return;

  _resource = std::shared_ptr<TTF_Font>(TTF_OpenFont(_pathToResource.c_str(), 25), TTF_CloseFont);
  if (_resource) _loaded = true;
}


template <> void Resource<GLTexture>::Load()
{
  if (_loaded) return;
  _resource = std::shared_ptr<GLTexture>(new GLTexture);
  if (_resource)
  {
    _resource->LoadFromFile(_pathToResource);
    if (_resource->ID())
    {
      _loaded = true;
    }
  }
}
