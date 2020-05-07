#pragma once
#include "Geometry.h"
#include "AssetManagement/BlitOperation.h"
#include "AssetManagement/Animation.h"

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
  void Destroy() { _loadedTextures.clear(); }
  //! Initialize the main resource path
  void Initialize();
  //! Loads texture if unloaded and returns the SDL_Texture resource
  Texture& GetTexture(const std::string& file);
  //! Loads font from .ttf file and returns resource
  Font& GetFont(const std::string& file);
  //!
  TextResource& GetText(const char* text, const std::string& fontFile);
  //! Uses SDLQuery to get the width and height of the source texture
  Vector2<int> GetTextureWidthAndHeight(const std::string& file);
  //! Used by drawn objects to pass their drawing parameters to the resource manager
  BlitOperation* GetAvailableOp() { return &_registeredSprites[opIndex++]; }
  //! Adds a new blit op to the list. Only objects registered here will be drawn
  void RegisterBlitOp();
  //!
  void DeregisterBlitOp();
  //! Preps all the sprites to be presented on screen on the next SDL_RenderPresent call
  void BlitSprites();
  //! Gets the relative source path for the resources
  const std::string& GetResourcePath() { return _resourcePath; }

  static void CrawlTexture(Texture& texture, Vector2<int> begin, Vector2<int> end, std::function<void(int, int, Uint32)> callback);
  static Rect<double> FindRect(Texture& texture, Vector2<int> frameSize, Vector2<int> begPx);

private:

  //! All loaded texture resources
  std::unordered_map<std::string, Texture> _loadedTextures;
  //! All loaded font resources
  std::unordered_map<std::string, Font> _loadedFonts;
  //! All loaded font resources
  std::unordered_map<std::string, TextResource> _loadedTexts;
  //! Number of sprites that will be drawn in the scene
  int registeredSprites = 0;
  //! Index of the latest available op spot
  int opIndex = 0;
  //! All registered blit ops. Trying to use spatial loading to make drawing faster when there are a lot of object on screen
  std::vector<BlitOperation> _registeredSprites;
  //! Relative source path for all of the resources
  std::string _resourcePath;

  //!
  ResourceManager() = default;
  ResourceManager(const ResourceManager&) = delete;
  ResourceManager operator=(ResourceManager&) = delete;

};
