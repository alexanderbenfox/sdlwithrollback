#pragma once

#include <unordered_map>
#include "AssetManagement/Resource.h"

class ResourceManager
{
public:
  static ResourceManager& Get()
  {
    static ResourceManager rm;
    return rm;
  }

  void Initialize();

  void LoadFile(const std::string& file);

private:
  ResourceManager();

  std::unordered_map<std::string, Texture> _loadedTextures;
  std::unordered_map<std::string, Font> _font;

  std::string _dataPath;

};

class GameManager
{
public:
  static GameManager& Get()
  {
    static GameManager gm;
    return gm;
  }
  //!
  bool Ready() { return _initialized; }
  //!
  void Initialize();
  //!
  void Destroy();
  //!
  void BeginGameLoop();

  SDL_Renderer* GetRenderer() { return _renderer; }

private:
  GameManager() : _initialized(false) {}
  ~GameManager();

  void Update(int deltaTime_ms);
  void Draw();

  bool _initialized;

  bool _playing;

  SDL_Renderer* _renderer;
  SDL_Window* _window;
};