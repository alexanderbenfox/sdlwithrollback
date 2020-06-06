#pragma once
#include "Geometry.h"
#include "AssetManagement/LetterCase.h"

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
  void Destroy() {}
  //! Initialize the main resource path
  void Initialize();
  //! Loads texture if unloaded and returns the SDL_Texture resource
  template <typename AssetType>
  Resource<AssetType>& GetAsset(const std::string& file);
  //!
  TextResource& GetText(const char* text, const std::string& fontFile);
  //!
  LetterCase& GetFontWriter(const std::string& fontFile, size_t size);
  //! Uses SDLQuery to get the width and height of the source texture
  Vector2<int> GetTextureWidthAndHeight(const std::string& file);
  //! Gets the relative source path for the resources
  const std::string& GetResourcePath() { return _resourcePath; }

  //////++++ WE NEED TO GET THIS OUTTA HERE - BEGIN
  //!
  void RenderGL();
  //!
  void AppendDraw(const std::vector<GLDrawOperation>& toDraw)
  {
    _glTexturesToDraw.insert(_glTexturesToDraw.end(), toDraw.begin(), toDraw.end());
  }
  static void CrawlTexture(Resource<SDL_Texture>& texture, Vector2<int> begin, Vector2<int> end, std::function<void(int, int, Uint32)> callback);
  static Rect<double> FindRect(Resource<SDL_Texture>& texture, Vector2<int> frameSize, Vector2<int> begPx);
  //////++++ WE NEED TO GET THIS OUTTA HERE - END

private:

  //! All loaded assets from file resources
  template <typename AssetType>
  static std::unordered_map<std::string, Resource<AssetType>> _fileAssets;
  //! Relative source path for all of the resources
  std::string _resourcePath;

  //////++++ WE NEED TO GET THIS OUTTA HERE - BEGIN
 //! All loaded font resources
  std::unordered_map<std::string, TextResource> _loadedTexts;
  //! Loaded FONT resources
  std::unordered_map<FontKey, LetterCase> _loadedLetterCases;
  std::vector<GLDrawOperation> _glTexturesToDraw;
  //////++++ WE NEED TO GET THIS OUTTA HERE - END

  //!
  ResourceManager() = default;
  ResourceManager(const ResourceManager&) = delete;
  ResourceManager operator=(ResourceManager&) = delete;

};
