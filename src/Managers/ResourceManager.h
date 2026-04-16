#pragma once
#include "Core/Utility/String.h"
#include "Core/Geometry2D/Rect.h"
#include "AssetManagement/LetterCase.h"
#include "AssetManagement/EditableAssets/SpriteSheet.h"

#include <unordered_map>
#include <vector>
#include <functional>

//______________________________________________________________________________
//! Manager of resources for textures, font, sounds, and drawing
class ResourceManager
{
public:
  //! Returns singleton instance of the Resource Manager
  static ResourceManager& Get() { static ResourceManager rm; return rm; }
  //! Destroy loaded resource objects
  void Destroy();
  //! Initialize the main resource path
  void Initialize();
  //! Loads texture if unloaded and returns the SDL_Texture resource
  template <typename AssetType>
  Resource<AssetType>& GetAsset(const std::string& file);
  //!
  LetterCase& GetFontWriter(const std::string& fontFile, size_t size);
  //! Uses SDLQuery to get the width and height of the source texture
  Vector2<int> GetTextureWidthAndHeight(const std::string& file);
  //! Gets the relative source path for the resources
  const std::string& GetResourcePath() { return _resourcePath; }

  AssetLibrary<SpriteSheet> gSpriteSheets;

private:

  //! All loaded assets from file resources
  template <typename AssetType>
  static std::unordered_map<std::string, Resource<AssetType>> _fileAssets;
  //! Loaded FONT resources
  std::unordered_map<FontKey, LetterCase> _loadedLetterCases;
  //! Relative source path for all of the resources
  std::string _resourcePath;

  //!
  ResourceManager() = default;
  ResourceManager(const ResourceManager&) = delete;
  ResourceManager operator=(ResourceManager&) = delete;

};

template <typename AssetType>
std::unordered_map<std::string, Resource<AssetType>> ResourceManager::_fileAssets{};

//______________________________________________________________________________
template <typename AssetType>
inline Resource<AssetType>& ResourceManager::GetAsset(const std::string& file)
{
  if (_fileAssets<AssetType>.find(file) == _fileAssets<AssetType>.end())
  {
    _fileAssets<AssetType>.emplace(std::piecewise_construct, std::make_tuple(file), std::make_tuple(_resourcePath + file));
  }
  _fileAssets<AssetType>[file].Load();
  return _fileAssets<AssetType>[file];
}
