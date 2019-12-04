#include "AssetManagement/Resource.h"
#include "GameManagement.h"

#include "SDL.h"

template <typename T>
void Resource<T>::Unload()
{
  _resource.reset();
  _resource = nullptr;
}

template <> void Resource<SDL_Texture>::Load()
{
  if (_loaded) return;

  //auto surface = ResourceManager::Get().GetRawImage(_pathToResource.c_str());
  SDL_Surface* surface = IMG_Load(_pathToResource.c_str());
  if (surface)
  {
    SDL_Texture* tex = SDL_CreateTextureFromSurface(GameManager::Get().GetRenderer(), surface);
    _resource = std::shared_ptr<SDL_Texture>(tex, SDL_DestroyTexture);
    if(_resource) _loaded = true;
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
