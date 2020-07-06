#pragma once

#include "GameState/GameState.h"
#include "GameState/Scene.h"
#include "Components/IComponent.h"
#include "Timer.h"
#include "Rendering/RenderManager.h"

#include <thread>
#include <mutex>

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
  //! Gets the result of the latest SDL_PollEvent
  SDL_Event const& GetLocalInput() { return _localInput; }

  std::shared_ptr<Entity> GetEntityByID(int id) { return _gameEntities[id]; }

  void CheckAgainstSystems(Entity* entity);

  void ActivateHitStop(int frames);

  void DebugDraws();

  void DestroyEntity(std::shared_ptr<Entity> entity);

  

  //! request scene change at end of update loop
  void RequestSceneChange(SceneType newSceneType);
  //! Schedules a function to be ran when scene is changed
  void TriggerOnSceneChange(std::function<void()> function)
  {
    _onSceneChangeFunctionQueue.push_back(function);
  }

  //! Schedules some functor to be called at the end of the update loop
  void TriggerEndOfFrame(std::function<void()> function)
  {
    _endOfFrameQueue.push_back(function);
  }

  


private:
  void ChangeScene(SceneType scene);
  //! Updates all components in specified order
  void Update(float deltaTime);
  //!
  void PostUpdate();
  //! Updates SDL input based on the SDL event system
  void UpdateInput();
  //! Flushes last render frame, draws all objects on the screen, displays all drawn objects
  void Draw();
  //! Flag for whether or not the GM has been initialized
  bool _initialized;
  //!
  SDL_Event _localInput;
  //!
  Timer _clock;

  //!
  std::unique_ptr<IScene> _currentScene;
  //
  SceneType _currentSceneType;
  //
  bool _sceneChangeRequested = false;
  //
  std::vector<std::function<void()>> _onSceneChangeFunctionQueue, _endOfFrameQueue;
  

  //______________________________________________________________________________

  //! Helper function for adding multiple components to an entity at one time
  template <typename T = IComponent, typename ... Rest>
  static auto AddComponentToEntity(Entity* entity) -> std::enable_if_t<!std::is_void<T>::value>;
  //! Helper function for adding multiple components to an entity at one time
  template <typename T = IComponent, typename ... Rest>
  static auto ComponentExistsOnEntity(Entity* entity);// -> std::enable_if_t<!std::is_void<T>::value>;
  //! All entities in the scene
  std::vector<std::shared_ptr<Entity>> _gameEntities;
  std::shared_ptr<Entity> _p1, _p2;
  //______________________________________________________________________________
  //!
  GameManager();
  GameManager(const GameManager&) = delete;
  GameManager(GameManager&&) = delete;
  GameManager operator=(const GameManager&) = delete;
  GameManager operator=(GameManager&&) = delete;

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
