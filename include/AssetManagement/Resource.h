#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <string>
#include <memory>

template <typename T>
struct ResourceTraits {};

template <> struct ResourceTraits<SDL_Texture>
{
  ResourceTraits() : mPitch(0), mWidth(0), mHeight(0) {}
  int mPitch, mWidth, mHeight;
  std::unique_ptr<unsigned char> pixels;
  Uint32 transparent;
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

  virtual void Load();
  void Unload();

  T* Get() { return _resource.get(); }
  bool IsLoaded() {return _loaded;}

  ResourceTraits<T>& GetInfo() { return _info; }

protected:
  bool _loaded;
  std::string _pathToResource;
  std::shared_ptr<T> _resource;
  ResourceTraits<T> _info;

};

//template <> void Resource<SDL_Texture>::Load();
//template <> void Resource<TTF_Font>::Load();
