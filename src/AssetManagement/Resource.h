#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "Rendering/GLTexture.h"

#include <string>
#include <memory>

template <typename T>
struct ResourceTraits {};

template <typename T> class Resource;

template <> struct ResourceTraits<SDL_Texture>
{
  ResourceTraits() : mPitch(0), mWidth(0), mHeight(0) {}
  int mPitch, mWidth, mHeight;
  std::unique_ptr<unsigned char> GetPixels(const std::string& resourcePath) const;
  Uint32 transparent;

};

template <> struct ResourceTraits<GLTexture>
{
  ResourceTraits() : mPitch(0), mWidth(0), mHeight(0) {}
  int mPitch, mWidth, mHeight;

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
  Resource(std::shared_ptr<T>&& ptr) : _loaded(true)
  {
    _resource = std::move(ptr);
  }

  virtual ~Resource()
  {
    if (_loaded)
      Unload();
  }

  virtual void Load();
  virtual void Unload()
  {
    _resource.reset();
    _resource = nullptr;
    _loaded = false;
  }

  T* Get() { return _resource.get(); }
  const T* GetConst() const { return _resource.get(); }
  bool IsLoaded() {return _loaded;}
  std::string GetPath() { return _pathToResource; }

  ResourceTraits<T> const& GetInfo() const { return _info; }

  T* operator->() const { return _resource.get(); }

protected:
  bool _loaded;
  std::string _pathToResource;
  std::shared_ptr<T> _resource;
  ResourceTraits<T> _info;

};

template <typename T>
class ResourceWrapper
{
public:
  ResourceWrapper(Resource<T>& resource) : _resource(resource) {}
  Resource<T>& GetResource() { return _resource; }
private:
  Resource<T>& _resource;
};

//template <> void Resource<SDL_Texture>::Load();
//template <> void Resource<TTF_Font>::Load();
