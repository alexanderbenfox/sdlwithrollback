#include "AssetManagement/Resource.h"
#include "Managers/GameManagement.h"

#include <SDL2/SDL.h>

#include <iostream>

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


template <> void Resource<BgfxTexture>::Load()
{
  if (_loaded) return;
  _resource = std::shared_ptr<BgfxTexture>(new BgfxTexture);
  if (_resource)
  {
    _resource->LoadFromFile(_pathToResource);
    if (bgfx::isValid(_resource->Handle()))
    {
      _info.mWidth = _resource->w();
      _info.mHeight = _resource->h();
      _info.mPitch = _resource->w() * 4;
      _loaded = true;
    }
  }
}
