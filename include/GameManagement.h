#pragma once

#include <unordered_map>
#include "AssetManagement/Resource.h"
#include "Entity.h"
#include "Components/IComponent.h"

class IInputHandler;

class GameActor;
class Camera;

typedef Resource<SDL_Texture> Texture;
typedef Resource<TTF_Font> Font;

//______________________________________________________________________________
//! Manager of resources for textures, font, sounds, and drawing
class ResourceManager
{
public:
  //! Drawing parameters drawing sprite objects
  struct BlitOperation
  {
    bool valid = false;
    SDL_Rect _textureRect;
    SDL_Rect _displayRect;
    Texture* _textureResource;
    SDL_RendererFlip _flip;
  };

  //! Returns singleton instance of the Resource Manager
  static ResourceManager& Get() { static ResourceManager rm; return rm; }
  //! Destroy loaded resource objects
  void Destroy() { _loadedTextures.clear(); }
  //! Initialize the main resource path
  void Initialize();
  //! Loads texture if unloaded and returns the SDL_Texture resource
  Texture& GetTexture(const std::string& file);
  //! Uses SDLQuery to get the width and height of the source texture
  Vector2<int> GetTextureWidthAndHeight(const std::string& file);
  //! Used by drawn objects to pass their drawing parameters to the resource manager
  BlitOperation* GetAvailableOp() { return &_registeredSprites[opIndex++]; }
  //! Adds a new blit op to the list. Only objects registered here will be drawn
  void RegisterBlitOp();
  //! Preps all the sprites to be presented on screen on the next SDL_RenderPresent call
  void BlitSprites();
  //! Gets the relative source path for the resources
  const std::string& GetResourcePath() { return _resourcePath; }

private:
  
  //! All loaded texture resources
  std::unordered_map<std::string, Texture> _loadedTextures;
  //! All loaded font resources
  std::unordered_map<std::string, Font> _font;
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

//______________________________________________________________________________
//! Manager of all things related to whats happening in the game
class GameManager
{
public:
  //! Get Singleton instance of the GameManager
  static GameManager& Get() { static GameManager gm; return gm; }
  //! Checks if the Game Manager has been initialized
  bool Ready() const { return _initialized; }
  //! Initialize the game manager, the SDL Library, and all game entities in the scene
  void Initialize();
  //! Cleans up all SDL subsystems and destroys objects in correct order
  void Destroy();
  //! Starts the game loop. Returns when the game has been ended
  void BeginGameLoop();
  //! Returns a pointer to the SDL_Renderer object held by the game manager
  SDL_Renderer* GetRenderer() { return _renderer; }
  //! Gets the camera used by the rendering pipeline to cull game entities
  Camera* GetMainCamera() { return _mainCamera.get(); }

private:
  //! Updates all components in specified order
  void Update(float deltaTime);
  //! Checks for input based on the event and IInputHandler. If an event is received, send it to the player controlled game actor
  void UpdateInput(SDL_Event* event);
  //! Flushes last render frame, draws all objects on the screen, displays all drawn objects
  void Draw();
  //! Flag for whether or not the GM has been initialized
  bool _initialized;
  //! SDL Renderer
  SDL_Renderer* _renderer;
  //! Window object
  SDL_Window* _window;
  //! Player input listener
  std::unique_ptr<IInputHandler> _playerInput;

  //______________________________________________________________________________
  template <typename ...> struct ComponentInitList {};

  template <typename T = IComponent, typename ... Rest>
  void AddComponentToEntity(Entity* entity, ComponentInitList<T, Rest...> )
  {
    entity->AddComponent<T>();

    // recursively call this function on the rest of the types
    AddComponentToEntity(entity, ComponentInitList<Rest...>());
  }

  template <typename ... Args>
  std::shared_ptr<Entity> CreateEntity()
  {
      //static constexpr unsigned nargs = sizeof...(Args);
      _gameEntities.push_back(std::make_shared<Entity>());
      auto newEntity = _gameEntities.back();
      AddComponentToEntity(newEntity.get(), ComponentInitList<Args...>());
      return newEntity;
  }

  //! All entities in the scene
  std::vector<std::shared_ptr<Entity>> _gameEntities;
  //! Currently controller object
  std::shared_ptr<GameActor> _player;
  //! Camera used by rendering pipeline to view the scene
  std::shared_ptr<Camera> _mainCamera;

  //______________________________________________________________________________
  //!
  GameManager();
  GameManager(const GameManager&) = delete;
  GameManager operator=(GameManager&) = delete;

};

//______________________________________________________________________________
//!
class IGameState
{
public:
  virtual ~IGameState() = 0;
  virtual void LoadAssets() = 0; 
};
