#pragma once

#include "GameState/SceneTypes.h"
#include "Core/ECS/Entity.h"
#include "Core/Timer.h"
#include "Rendering/RenderManager.h"
#include "Core/InputState.h"

class IScene;

#include <thread>
#include <mutex>

#define GRenderer RenderManager::Get()

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

  void ActivateHitStop(int frames);

  void DebugDraws();

  //______________________________________________________________________________
  //! Add entity to game entity list and add components to it
  template <class ... Args>
  std::shared_ptr<Entity> CreateEntity();
  std::shared_ptr<Entity> GetEntityByID(int id) { return _gameEntities[id]; }
  void DestroyEntity(std::shared_ptr<Entity> entity);
  void DestroyEntity(const EntityID& entity);
  void AddToNetworkedList(const EntityID& entity) { _networkedEntities.push_back(entity); }
  std::shared_ptr<Entity> GetP1() const { return _p1; }
  std::shared_ptr<Entity> GetP2() const { return _p2; }

  //! request scene change at end of update loop
  void RequestSceneChange(SceneType newSceneType);
  //!
  void AdvanceCurrentScene();
  //!
  void SetBattleType(BattleType type) { _currentBattleType = type; }
  //! Schedules a function to be ran when scene is changed
  void TriggerOnSceneChange(std::function<void()> fn)
  {
    _onSceneChangeFunctionQueue.push_back(fn);
  }

  //! Schedules some functor to be called at the end of the update loop
  void TriggerEndOfFrame(std::function<void()> fn)
  {
    _endOfFrameQueue.push_back(fn);
  }

  void TriggerBeginningOfFrame(std::function<void()> fn)
  {
    _beginningOfFrameQueue.push_back(fn);
  }

  //! Creates a snapshot of all of the current entities' states (prepending the EntityID before each entity state is written)
  SBuffer CreateGameStateSnapshot() const;
  //! Loads the snapshot of the current game state
  void LoadGamestateSnapshot(const SBuffer& snapshot);
  //! 
  std::string LogGamestate();

  //! Updates all components in specified order
  void Update(float deltaTime);
  //! Updates player input after a sync
  void SyncPlayerInputs(InputState* inputs);


private:
  //! Collects inputs and runs update function
  void RunFrame(float deltaTime);
  //!
  void ChangeScene(SceneType scene);
  //!
  void PostUpdate();
  //! Updates SDL input based on the SDL event system
  SDL_Event UpdateLocalInput();
  //! Flushes last render frame, draws all objects on the screen, displays all drawn objects
  void Draw();
  //! Destroys marked entities and clears scene change queue
  void ClearSceneData();
  //! Flag for whether or not the GM has been initialized
  bool _initialized;
  //!
  Timer _clock;

  //!
  bool _frameStopActive = false;
  //!
  int _frameStop = 0;

  ////!!!!! SCENE CODE
  //!
  std::unique_ptr<IScene> _currentScene;
  //
  SceneType _currentSceneType;
  //
  BattleType _currentBattleType = BattleType::Training;
  //
  bool _sceneChangeRequested = false;

  //
  std::vector<std::function<void()>> _onSceneChangeFunctionQueue, _endOfFrameQueue, _beginningOfFrameQueue;

  SDL_Event _hardwareEvents;

  bool _running;


  //! All entities in the scene
  std::unordered_map<EntityID, std::shared_ptr<Entity>> _gameEntities;
  //! Game state dependent entities that must be transferred by the network
  std::vector<EntityID> _networkedEntities;
  //!
  std::shared_ptr<Entity> _p1, _p2;


  //______________________________________________________________________________
  //!
  GameManager();
  ~GameManager();
  GameManager(const GameManager&) = delete;
  GameManager(GameManager&&) = delete;
  GameManager operator=(const GameManager&) = delete;
  GameManager operator=(GameManager&&) = delete;

};

//______________________________________________________________________________
template <class ... Args>
inline std::shared_ptr<Entity> GameManager::CreateEntity()
{
  auto entityPtr = std::make_shared<Entity>();
  _gameEntities[entityPtr->GetID()] = entityPtr;

  entityPtr->AddComponents<Args...>();
  return entityPtr;
}
