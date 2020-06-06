#pragma once

#include "GameState/GameState.h"
#include "Entity.h"
#include "Components/IComponent.h"
#include "Timer.h"
#include "Rendering/RenderManager.h"

#include <thread>
#include <mutex>

class Camera;

template <typename T, typename... Rest>
bool constexpr all_base_of()
{
  return (std::is_base_of_v<T, Rest> && ...);
}

//! Set our preferred type (SDL or GL) to be rendered by the system
typedef SDL_Texture RenderType;
#define GRenderer RenderManager<RenderType>::Get()

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
  //! Gets the camera used by the rendering pipeline to cull game entities
  Camera* GetMainCamera();
  //! Add entity to game entity list and add components to it
  template <class ... Args>
  std::shared_ptr<Entity> CreateEntity();

  SDL_Event const& GetLocalInput()
  {
    return _localInput;
  }

  std::shared_ptr<Entity> GetEntityByID(int id) { return _gameEntities[id]; }

  void CheckAgainstSystems(Entity* entity);

  void ActivateHitStop(int frames);

  const int hitstopLight = 10;
  const int hitstopMedium = 10;
  const int hitstopHeavy = 10;

private:
  //! Updates all components in specified order
  void Update(float deltaTime);
  //! Checks for input based on the event and IInputHandler. If an event is received, send it to the player controlled game actor
  void UpdateInput();
  //! Flushes last render frame, draws all objects on the screen, displays all drawn objects
  void Draw();
  //!
  void RunScripter(std::thread& t, bool& programRunning);
  //! Flag for whether or not the GM has been initialized
  bool _initialized;
  //!
  SDL_Event _localInput;
  //!
  Timer _clock;
  //!
  std::mutex _debugMutex;
  //!
  //std::unique_ptr<IGameState> _gameState;
  std::shared_ptr<Camera> _camera; 

  //______________________________________________________________________________

  //! Helper function for adding multiple components to an entity at one time
  template <typename T = IComponent, typename ... Rest>
  static auto AddComponentToEntity(Entity* entity) -> std::enable_if_t<!std::is_void<T>::value>;
  //! Helper function for adding multiple components to an entity at one time
  template <typename T = IComponent, typename ... Rest>
  static auto ComponentExistsOnEntity(Entity* entity);// -> std::enable_if_t<!std::is_void<T>::value>;
  //! All entities in the scene
  std::vector<std::shared_ptr<Entity>> _gameEntities;
  //______________________________________________________________________________
  //!
  GameManager();
  GameManager(const GameManager&) = delete;
  GameManager operator=(GameManager&) = delete;

};

//______________________________________________________________________________
template <typename T, typename ... Rest>
inline auto GameManager::AddComponentToEntity(Entity* entity) -> std::enable_if_t<!std::is_void<T>::value>
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
template <typename T, typename ... Rest>
inline auto GameManager::ComponentExistsOnEntity(Entity* entity)
{
  // recursive control path enders
  if (!all_base_of<IComponent, T, Rest...>() || std::is_same_v<T, IComponent>)
    return true;
  return entity->GetComponent<T>() && ComponentExistsOnEntity<Rest...>(entity);
}

//______________________________________________________________________________
template <class ... Args>
inline std::shared_ptr<Entity> GameManager::CreateEntity()
{
  _gameEntities.push_back(std::make_shared<Entity>());
  auto newEntity = _gameEntities.back();
  AddComponentToEntity<Args...>(newEntity.get());
    // this is just a test... will be moved later
  CheckAgainstSystems(newEntity.get());

  return newEntity;
}
