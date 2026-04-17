#include "Managers/GameManagement.h"

#include <iostream>

#include "Systems/DrawCallSystems.h"
#include "Systems/DestroyEntitiesSystem.h"
#include "DebugGUI/GUIController.h"
#include "DebugGUI/DebugSetup.h"

#include "GameState/Scene.h"
#include "GameState/MatchScene.h"

#include "Components/Actors/GameActor.h"
#include "Components/Input.h"
#include "Components/Hitbox.h"
#include "Components/Hurtbox.h"
#include "Components/ActionComponents.h"
#include "Managers/AnimationCollectionManager.h"
#include "Managers/GGPOManager.h"

#include "Core/Utility/Profiler.h"

#include <sstream>

#ifdef _DEBUG
//used for debugger
#include <thread>
#include <iostream>
#endif

//______________________________________________________________________________
void GameManager::Initialize()
{
  GRenderer.Init();

  //! Call this to initialize animation collections and load them
  AnimationCollectionManager::Get();

  //! initialize our player controllable entities
  _p1 = CreateEntity<GameInputComponent, Actor>();
  _p2 = CreateEntity<GameInputComponent, Actor>();

  AddToNetworkedList(_p1->GetID());
  AddToNetworkedList(_p2->GetID());

  //! Initialize them with keyboard handlers
  _p1->GetComponent<GameInputComponent>()->AssignHandler(InputType::Keyboard);
  //! Initialize second player as defend all AI
  _p2->GetComponent<GameInputComponent>()->AssignHandler(InputType::DefendAfter);

  //! initialize the scene
  ChangeScene(SceneType::START);

  _initialized = true;
}

//______________________________________________________________________________
void GameManager::Destroy()
{
  // Release player refs before clearing — they also hold shared_ptrs to entities
  _p1.reset();
  _p2.reset();

  // Clear entities before renderer so GL resources are freed while context is still valid
  _gameEntities.clear();
  _networkedEntities.clear();

  GRenderer.Destroy();
}

//______________________________________________________________________________
void GameManager::BeginGameLoop()
{
  if constexpr (std::is_same_v<RenderType, SDL_Texture>)
  {
    GUIController::Get().InitSDLWindow();
    GUIController::Get().InitImGUI();
  }
  else
  {
    // if we're using gl to render our window, just render imgui in our window
    GUIController::Get().InitImGUI(GRenderer.GetWindow(), GRenderer.GetGLContext());
  }

  AvgCounter tracker;
  SetupDebugWindows(*this, _clock, tracker);

  //start the timer at 60 fps
  _clock.Start(60);

  //initialize the functions
  UpdateFunction update = std::bind(&GameManager::RunFrame, this, std::placeholders::_1);

  int frameCount = 0;
  _running = true;

  while(_running)
  {
    if (frameCount == 0)
      tracker.Add(_clock.GetUpdateTime());

    //! Update all components and coroutines
    _clock.Update(update);
    //! Update gui
    GUIController::Get().MainLoop();
    //! render the scene
    Draw();

    frameCount = (++frameCount) % 10;
  }
  // destroy all entities in the scene before cleaning up gui
  _currentScene.reset();
  GUIController::Get().CleanUp();
}

//______________________________________________________________________________
void GameManager::ActivateHitStop(int frames)
{
  _frameStop = frames;
  _frameStopActive = true;
}

//______________________________________________________________________________
void GameManager::DebugDraws()
{
  // draw items in debug layer over top of the drawn scene
  ComponentArray<Hurtbox>::Get().ForEach([](Hurtbox& hb) { hb.Draw(); });
  ComponentArray<Hitbox>::Get().ForEach([](Hitbox& hb) { hb.Draw(); });
  ComponentArray<ThrowFollower>::Get().ForEach([](ThrowFollower& tb) { tb.Draw(); });
}

//______________________________________________________________________________
void GameManager::DestroyEntity(std::shared_ptr<Entity> entity)
{
  DestroyEntity(entity->GetID());
}

void GameManager::DestroyEntity(const EntityID& entity)
{
  auto it = _gameEntities.find(entity);
  if (it != _gameEntities.end())
  {
    // delete from networked entities list 
    auto nIt = std::find(_networkedEntities.begin(), _networkedEntities.end(), entity);
    if (nIt != _networkedEntities.end())
    {
      _networkedEntities.erase(nIt);
    }

    it->second->RemoveAllComponents();
    _gameEntities.erase(it);
  }
}

//______________________________________________________________________________
void GameManager::RequestSceneChange(SceneType newSceneType)
{
  _sceneChangeRequested = true;
  _currentSceneType = newSceneType;
}

//______________________________________________________________________________
void GameManager::AdvanceCurrentScene()
{
  _beginningOfFrameQueue.push_back([this]()
  {
    ClearSceneData();
    _currentScene->AdvanceScene();
  });
}

//______________________________________________________________________________
SBuffer GameManager::CreateGameStateSnapshot() const
{
  std::stringstream stream;

  // maybe serialize some metadata here?
  Serializer<SceneType>::Serialize(stream, _currentSceneType);

  // serialize frame stop data here
  Serializer<bool>::Serialize(stream, _frameStopActive);
  Serializer<int>::Serialize(stream, _frameStop);

  for (const EntityID& id : _networkedEntities)
  {
    Serializer<EntityID>::Serialize(stream, id);
    _gameEntities.at(id)->Serialize(stream);
  }

  return SBuffer(std::istreambuf_iterator<char>(stream), {});
}

//______________________________________________________________________________
void GameManager::LoadGamestateSnapshot(const SBuffer& snapshot)
{
  // write contents to the stream
  std::stringstream stream;
  stream.write((const char*)snapshot.data(), snapshot.size());

  // get the scene the snapshot was written in
  SceneType snapshotScene;
  Serializer<SceneType>::Deserialize(stream, snapshotScene);
  assert(_currentSceneType == snapshotScene);

  Serializer<bool>::Deserialize(stream, _frameStopActive);
  Serializer<int>::Deserialize(stream, _frameStop);

  std::vector<EntityID> nonLoadedEntities = _networkedEntities;
  while (!stream.eof())
  {
    EntityID cpID = 0;
    Serializer<EntityID>::Deserialize(stream, cpID);

    if (stream.eof())
      break;

    std::shared_ptr<Entity> entity = nullptr;

    // if this has already been destroyed, create new entity because it could possibly be a fireball or something
    // but maybe it should still throw a warning
    if (_gameEntities.find(cpID) == _gameEntities.end())
    {
      entity = CreateEntity<>();
      std::cerr << "Attempting to load gamestate with inaccessible entities. Be careful entities are not being copied.\n";

      // add to networked list... probably need to sync this
      AddToNetworkedList(entity->GetID());
    }
    else
    {
      entity = _gameEntities[cpID];
    }

    // finally load the entire component state into the entity
    entity->Deserialize(stream);

    // remove from list of entities
    auto it = std::find(nonLoadedEntities.begin(), nonLoadedEntities.end(), cpID);
    if (it != nonLoadedEntities.end())
      nonLoadedEntities.erase(it);
  }

  for (const EntityID& id : nonLoadedEntities)
    DestroyEntity(id);
}

//______________________________________________________________________________
std::string GameManager::LogGamestate()
{
  std::string s = "";
  for (const EntityID& id : _networkedEntities)
  {
    s += _gameEntities.at(id)->Log();
  }
  return s;
}

//______________________________________________________________________________
void GameManager::Update(float deltaTime)
{
  // makes a profile for the function its contained in
  PROFILE_FUNCTION();

  _currentScene->Update(_frameStopActive ? 0.0f : deltaTime);
  //! Do post update (update gui and change scene)
  PostUpdate();

  // end of frame decrement frame stop if necessary
  if (_frameStopActive)
  {
    _frameStop--;
    if (_frameStop <= 0)
      _frameStopActive = false;
  }

#ifdef _WIN32
  GGPOManager::Get().NotifyAdvanceFrame();
#endif
}

//______________________________________________________________________________
void GameManager::SyncPlayerInputs(InputState* inputs)
{
  _p1->GetComponent<GameInputComponent>()->PushState(inputs[0]);
  _p2->GetComponent<GameInputComponent>()->PushState(inputs[1]);
}

//______________________________________________________________________________
void GameManager::RunFrame(float deltaTime)
{
  PROFILE_SCOPE("RunFrame");

  if (!_beginningOfFrameQueue.empty())
  {
    for (auto& func : _beginningOfFrameQueue)
      func();
    _beginningOfFrameQueue.clear();
  }

  // defer scene change until end of update loops
  if (_sceneChangeRequested)
  {
    ChangeScene(_currentSceneType);
    _sceneChangeRequested = false;

    // HERE: avoid unwanted scheduled tasks running in new scene
    // added deferments from StateMachine systems are causing problems
    // on scene change
  }

  // grab events from hardware
  _hardwareEvents = UpdateLocalInput();
  // update debug gui logic
  GUIController::Get().UpdateLogic(_hardwareEvents);
  // translate events to input state
  InputState inputs[2];
  inputs[0] = _p1->GetComponent<GameInputComponent>()->TranslateEvent(_hardwareEvents);
  inputs[1] = _p2->GetComponent<GameInputComponent>()->TranslateEvent(_hardwareEvents);

  // if we're not online, we can just advance the frame
  bool advanceFrame = true;

#ifdef _WIN32
  if (GGPOManager::Get().InMatch())
  {
    if (!GGPOManager::Get().SyncInputs(inputs))
    {
      advanceFrame = false;
    }
  }
#endif // _WIN32

  // advance frame with correct inputs assigned
  if (advanceFrame)
  {
    _p1->GetComponent<GameInputComponent>()->PushState(inputs[0]);
    _p2->GetComponent<GameInputComponent>()->PushState(inputs[1]);

    Update(deltaTime);


  }
}

//______________________________________________________________________________
void GameManager::ChangeScene(SceneType scene)
{
  _currentScene.reset();
  ClearSceneData();

  _currentScene = std::unique_ptr<IScene>(SceneHelper::CreateScene(scene));
  if (scene == SceneType::MATCH)
  {
    ((MatchScene*)_currentScene.get())->_battleType = _currentBattleType;
  }

  _currentScene->Init(_p1, _p2);
  _currentSceneType = scene;

}

//______________________________________________________________________________
void GameManager::PostUpdate()
{
  if (!_endOfFrameQueue.empty())
  {
    for (auto& func : _endOfFrameQueue)
      func();
    _endOfFrameQueue.clear();
  }
}

//______________________________________________________________________________
SDL_Event GameManager::UpdateLocalInput()
{
  SDL_Event event;
  // Check for quit or resize and update input object
  if (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT)
    {
      _running = false;
    }
  }
  return event;
}

//______________________________________________________________________________
void GameManager::Draw()
{
  // send draw calls to the renderer
  SpriteDrawCallSystem::PostUpdate();
  UITextDrawCallSystem::PostUpdate();
  DrawUIPrimitivesSystem::PostUpdate();
  DrawUIBoxSpriteSystem::PostUpdate();

  //clear last frame graphics
  GRenderer.Clear();

  // draw the scene
  GRenderer.Draw();

  // draw debug imgui ui over that
  GUIController::Get().RenderFrame();

  //present this frame
  GRenderer.Present();
}

//______________________________________________________________________________
void GameManager::ClearSceneData()
{
  DestroyEntitiesSystem::DoTick(0.0);
  for (auto& func : _onSceneChangeFunctionQueue)
    func();
  _onSceneChangeFunctionQueue.clear();
}

//______________________________________________________________________________
GameManager::GameManager() : _initialized(false) {}

//______________________________________________________________________________
GameManager::~GameManager() = default;
