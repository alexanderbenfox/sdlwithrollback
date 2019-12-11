#pragma once

#include "SDL2/SDL.h"
#include "SDL2_image/SDL_image.h"
#include "SDL2/SDL_ttf.h"

#include <string>
#include <memory>

template <typename T>
class Resource
{
public:
  Resource() : _loaded(false) {}
  Resource(const std::string& filename) : _loaded(false)
  {
    _pathToResource = filename;
  }

  void Load();
  void Unload();

  T* Get() { return _resource.get(); }
  bool IsLoaded() {return _loaded;}

private:
  bool _loaded;
  std::string _pathToResource;
  std::shared_ptr<T> _resource;

};

//template <> void Resource<SDL_Texture>::Load();
//template <> void Resource<TTF_Font>::Load();
