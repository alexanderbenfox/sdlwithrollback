#include "../../include/AssetManagement/Resource.h"
#include "../../include/GameManagement.h"

template <typename T> Resource::Resource(const std::string& filename)
{
  _pathToResource = filename; 
}

template <> Resource<SDL_Texture>::Load()
{
  SDL_Surface* surface = IMG_Load(_pathToResource.c_str());
  if (surface)
  {
    _resource = std::shared_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(GameManager::Get().GetRenderer(), surface), SDL_DestroyTexture);
    SDL_FreeSurface(surface);
  }
}

template <> Resource<TTF_Font>::Load()
{
  _resource = std::shared_ptr<TTF_Font>(TTF_OpenFont(_pathToResource.c_str(), 25), TTF_CloseFont);
}

