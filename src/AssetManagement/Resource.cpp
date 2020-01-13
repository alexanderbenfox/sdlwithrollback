#include "AssetManagement/Resource.h"
#include "GameManagement.h"

#include <SDL2/SDL.h>

template <typename T>
void Resource<T>::Unload()
{
  _resource.reset();
  _resource = nullptr;
}

template <> void Resource<SDL_Texture>::Load()
{
  if (_loaded) return;

  Uint32 windowFormat = SDL_GetWindowPixelFormat(GameManager::Get().GetWindow());
  //Uint32 format = SDL_PIXELFORMAT_RGBA8888;

  //auto surface = ResourceManager::Get().GetRawImage(_pathToResource.c_str());
  SDL_Surface* surface = IMG_Load(_pathToResource.c_str());
  if (surface)
  {
    SDL_Surface* unformattedSurface = surface;
    surface = SDL_ConvertSurfaceFormat(surface, windowFormat, 0);
    SDL_FreeSurface(unformattedSurface);
    unformattedSurface = NULL;
  }
  if (surface)
  {
    SDL_Texture* texture = SDL_CreateTexture(GameManager::Get().GetRenderer(), windowFormat, SDL_TEXTUREACCESS_STREAMING, surface->w, surface->h);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    void* pixels;
     // Fill out information for the texture
    SDL_LockTexture(texture, NULL, &pixels, &_info.mPitch);
    // copy loaded pixels so they can be used again
    memcpy(pixels, surface->pixels, surface->pitch * surface->h);
    _info.pixels = std::unique_ptr<unsigned char>(new unsigned char[surface->pitch * surface->h]);
    memcpy(pixels, _info.pixels.get(), surface->pitch * surface->h);
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

//template <> class Resource<SDL_Texture>;
