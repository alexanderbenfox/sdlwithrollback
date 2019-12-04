#pragma once

#include <unordered_map>
#include "AssetManagement/Resource.h"
#include "Entity.h"

class IInputHandler;

class GameActor;
class Sprite;
class Camera;

typedef Resource<SDL_Texture> Texture;
typedef Resource<TTF_Font> Font;

class ResourceManager
{
public:
  struct BlitOperation
  {
    SDL_Rect _textureRect;
    SDL_Rect _displayRect;
    Texture* _textureResource;
    SDL_RendererFlip _flip;
  };

  static ResourceManager& Get()
  {
    static ResourceManager rm;
    return rm;
  }

  void Destroy()
  {
    _loadedTextures.clear();
  }

  void Initialize();

  Texture& GetTexture(const std::string& file);

  BlitOperation* RegisterBlitOp();

  void BlitSprites();

  const std::string& GetResourcePath() { return _resourcePath; }

private:
  ResourceManager() {}

  std::unordered_map<std::string, Texture> _loadedTextures;
  std::unordered_map<std::string, Font> _font;

  std::vector<BlitOperation> _registeredSprites;

  std::string _resourcePath;

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

  Camera* GetMainCamera() { return _mainCamera; }

private:
  GameManager();
  ~GameManager();

  void UpdateInput(SDL_Event* event);
  void Update(float deltaTime);
  void Draw();

  bool _initialized;

  SDL_Renderer* _renderer;
  SDL_Window* _window;

  std::unique_ptr<IInputHandler> _playerInput;

  //______________________________________
  std::vector<Entity> _gameEntities;

  GameActor* _player;
  Camera* _mainCamera;
};

class IGameState
{
public:
  virtual ~IGameState() = 0;
  virtual void LoadAssets() = 0; 
};