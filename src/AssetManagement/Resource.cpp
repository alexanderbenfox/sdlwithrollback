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
