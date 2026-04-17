#include "Managers/ResourceManager.h"
#include "AssetManagement/Resource.h"

#include <SDL.h>
#include <SDL_image.h>

//______________________________________________________________________________
void ResourceManager::Destroy()
{
}

//______________________________________________________________________________
void ResourceManager::Initialize()
{
  char* basePath = SDL_GetBasePath();
  if (basePath)
    _resourcePath = std::string(basePath) + "resources/";
  else _resourcePath = "./";
}

//______________________________________________________________________________
LetterCase& ResourceManager::GetFontWriter(const std::string& fontFile, size_t size)
{
  const FontKey key{ size, fontFile.c_str() };

  if (_loadedLetterCases.find(key) == _loadedLetterCases.end())
  {
    Resource<TTF_Font> font(_resourcePath + fontFile);
    font.Load();

    if(font.IsLoaded())
      _loadedLetterCases.emplace(std::piecewise_construct, std::make_tuple(key), std::make_tuple(font.Get(), size));
  }
  return _loadedLetterCases[key];
}

//______________________________________________________________________________
Vector2<int> ResourceManager::GetTextureWidthAndHeight(const std::string& file)
{
  SDL_Surface* surface = IMG_Load((_resourcePath + file).c_str());
  if (!surface)
    return Vector2<int>(0, 0);
  Vector2<int> size(surface->w, surface->h);
  SDL_FreeSurface(surface);
  return size;
}
