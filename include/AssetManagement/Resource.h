#pragma once

#include "SDL2/SDL.h"
#include "SDL2_image/SDL_image.h"
#include "SDL2/SDL_ttf.h"
#include <string>

template <typename T>
class Resource
{
public:
  Resource(const std::string& filename);
  ~Resource() {}

  void Load();
  void Unload();

  T* Get() { return _resource.get(); }
  bool IsLoaded() {return _loaded;}

private:
  bool _loaded;
  std::string _pathToResource;
  std::shared_ptr<T> _resource;
};

template <> Resource<SDL_Texture>::Resource(const std::string& filename);
template <> Resource<TTF_Font>::Resource(const std::string& filename);

typedef Resource<SDL_Texture> Texture;
typedef Resource<TTF_Font> Font;