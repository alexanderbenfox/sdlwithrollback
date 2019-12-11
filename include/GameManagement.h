#pragma once

#include "Entity.h"
#include "Input.h"

#include "Components/IComponent.h"

class GameActor;
class Camera;

template <typename T, typename... Rest>
bool constexpr all_base_of()
{
  return (std::is_base_of_v<T, Rest> && ...);
}

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

  //! Helper function for adding multiple components to an entity at one time
  template <typename T = IComponent, typename ... Rest>
  static auto AddComponentToEntity(Entity* entity) -> std::enable_if_t<!std::is_void<T>::value>;
  //! Add entity to game entity list and add components to it
  template <class ... Args>
  std::shared_ptr<Entity> CreateEntity();
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

//______________________________________________________________________________
template <typename T, typename ... Rest>
inline static auto GameManager::AddComponentToEntity(Entity* entity) -> std::enable_if_t<!std::is_void<T>::value>
{
  // recursive control path enders
  if (!all_base_of<IComponent, T, Rest...>() || std::is_same_v<T, IComponent>)
    return;

  // Add the current component
  entity->AddComponent<T>();
  // recursively call this function on the rest of the types
  return AddComponentToEntity<Rest...>(entity);
}

//______________________________________________________________________________
template <class ... Args>
inline std::shared_ptr<Entity> GameManager::CreateEntity()
{
  _gameEntities.push_back(std::make_shared<Entity>());
  auto newEntity = _gameEntities.back();
  AddComponentToEntity<Args...>(newEntity.get());
  return newEntity;
}
