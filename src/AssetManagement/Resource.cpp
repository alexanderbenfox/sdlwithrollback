#include "../../include/AssetManagement/Resource.h"
#include "../../include/GameManagement.h"

template <> Resource<SDL_Texture>::Resource(const std::string& filename)
{
  SDL_Surface* surface = IMG_Load(filename.c_str());
  if (surface)
  {
    _resource = std::shared_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(GameManager::Get().GetRenderer(), surface), SDL_DestroyTexture);
    SDL_FreeSurface(surface);
  }
}

template <> Resource<TTF_Font>::Resource(const std::string& filename)
{
  _resource = std::shared_ptr<TTF_Font>(TTF_OpenFont(filename.c_str(), 25), TTF_CloseFont);
}