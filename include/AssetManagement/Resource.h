#pragma once

#include "SDL2/SDL.h"
#include "SDL2_image/SDL_image.h"
#include "SDL2/SDL_ttf.h"

#include <string>
#include <memory>

template <typename T>
struct ResourceTraits {};

template <> struct ResourceTraits<SDL_Texture>
{
  int mPitch;
  int mWidth;
  int mHeight;
};

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

  ResourceTraits<T>& GetResourceInformation() { return _info; }

private:
  bool _loaded;
  std::string _pathToResource;
  std::shared_ptr<T> _resource;
  ResourceTraits<T> _info;

};

//template <> void Resource<SDL_Texture>::Load();
//template <> void Resource<TTF_Font>::Load();
