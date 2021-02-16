#include "Managers/GameManagement.h"

#include <iostream>

#include "Managers/ResourceManager.h"

#include "Systems/Physics.h"
#include "Systems/AnimationSystem.h"
#include "Systems/DrawCallSystems.h"
#include "Systems/MoveSystem.h"
#include "Systems/InputSystem.h"
#include "Systems/HitSystem.h"
#include "Systems/TimerSystem/TimerSystem.h"
#include "Systems/CutsceneSystem.h"
#include "Systems/CheckBattleEndSystem.h"
#include "Systems/UISystem.h"
#include "Systems/AISystem.h"
#include "Systems/DestroyEntitiesSystem.h"
#include "Systems/MenuSystem.h"
#include "Systems/WallPush/WallPushSystem.h"

#include "Systems/ActionSystems/EnactActionSystem.h"
#include "Systems/ActionSystems/ActionListenerSystem.h"
#include "Systems/ActionSystems/ActionHandleInputSystem.h"

#include "Systems/Physics.h"
#include "DebugGUI/GUIController.h"

#include "GameState/Scene.h"
#include "GameState/MatchScene.h"

#include "Components/Actors/GameActor.h"
#include "Managers/GGPOManager.h"

#include "AssetManagement/EditableAssets/Editor/AnimationEditor.h"
#include "AssetManagement/EditableAssets/AssetLibrary.h"

#include "Core/Utility/Profiler.h"

#include <sstream>

#ifdef _DEBUG
//used for debugger
#include <thread>
#include <iostream>
#endif

//______________________________________________________________________________
void ResourceManager::Destroy()
{
  _fileAssets<SDL_Texture>.clear();
}

//______________________________________________________________________________
void ResourceManager::Initialize()
{
  char* basePath = SDL_GetBasePath();
  if (basePath)
#ifdef _WIN32
    _resourcePath = std::string(basePath) + "..\\..\\..\\resources\\";
#else
    _resourcePath = std::string(basePath) + "resources/";
#endif
  else _resourcePath = "./";
}

//______________________________________________________________________________
TextResource& ResourceManager::GetText(const char* text, const std::string& fontFile)
{
  auto& font = GetAsset<TTF_Font>(fontFile);

  if (_loadedTexts.find(text) == _loadedTexts.end())
  {
    _loadedTexts.emplace(std::piecewise_construct, std::make_tuple(text), std::make_tuple(font.Get(), text, SDL_Color{ 255, 255, 255, SDL_ALPHA_OPAQUE }));
  }
  _loadedTexts[text].Load();
  return _loadedTexts[text];
}

//______________________________________________________________________________
LetterCase& ResourceManager::GetFontWriter(const std::string& fontFile, size_t size)
{
  const FontKey key{ size, fontFile.c_str() };

  if (_loadedLetterCases.find(key) == _loadedLetterCases.end())
  {
    Resource<TTF_Font> font(_resourcePath + fontFile);
    font.Load();

    if(font.IsLoaded())
      _loadedLetterCases.emplace(std::piecewise_construct, std::make_tuple(key), std::make_tuple(font.Get(), size));
  }
  return _loadedLetterCases[key];
}

//______________________________________________________________________________
Vector2<int> ResourceManager::GetTextureWidthAndHeight(const std::string& file)
{
  auto fileToQuery = file;
#ifndef _WIN32
  auto split = StringUtils::Split(file, '\\');
  if(split.size() > 1)
    fileToQuery = StringUtils::Connect(split.begin(), split.end(), '/');
#endif

  int width;
  int height;
  SDL_QueryTexture(GetAsset<SDL_Texture>(fileToQuery).Get(), nullptr, nullptr, &width, &height);
  return Vector2<int>(width, height);
}

//______________________________________________________________________________
void ResourceManager::CrawlTexture(Resource<SDL_Texture>& texture, Vector2<int> begin, Vector2<int> end, std::function<void(int, int, Uint32)> callback)
{
  auto& textureData = texture.GetInfo();
  // Get the window format
  Uint32 windowFormat = SDL_GetWindowPixelFormat(GRenderer.GetWindow());
  std::shared_ptr<SDL_PixelFormat> format = std::shared_ptr<SDL_PixelFormat>(SDL_AllocFormat(windowFormat), SDL_FreeFormat);

  // Get the pixel data
  Uint32* upixels;

#ifdef _WIN32
  auto px = textureData.GetPixels(texture.GetPath());
  upixels = (Uint32*)px.get();
  Uint32 transparent = textureData.transparent;
#else
  upixels = (Uint32*)textureData.pixels.get();
#endif
  for (int y = begin.y; y < end.y; y++)
  {
    for (int x = begin.x; x < end.x; x++)
    {
      callback(x - begin.x, y - begin.y, upixels[textureData.mWidth * y + x]);
    }
  }
}

//______________________________________________________________________________
Rect<double> ResourceManager::FindRect(Resource<SDL_Texture>& texture, Vector2<int> frameSize, Vector2<int> begPx)
{
  Uint32 windowFormat = SDL_GetWindowPixelFormat(GRenderer.GetWindow());
  std::shared_ptr<SDL_PixelFormat> format = std::shared_ptr<SDL_PixelFormat>(SDL_AllocFormat(windowFormat), SDL_FreeFormat);

  Uint32 transparent;
#ifdef _WIN32
  auto px = texture.GetInfo().GetPixels(texture.GetPath());
  Uint32* upixels = (Uint32*)px.get();
  transparent = texture.GetInfo().transparent;
#endif

  Vector2<int> rectBegin(-1, -1);
  Vector2<int> rectEnd(-1, -1);
  bool firstFound = false;

  auto buildRect = [format, &rectBegin, &rectEnd, &firstFound, &transparent](int x, int y, Uint32 pixel)
  {
#ifdef _WIN32
    if(pixel != transparent)
#else
    Uint8 r, g, b, a;
    SDL_GetRGBA(pixel, format.get(), &r, &g, &b, &a);
    if (r != 0 || b != 0 || g != 0 || a != 0)
#endif
    {
      if (!firstFound)
      {
        rectBegin = Vector2<int>(x, y);
        firstFound = true;
      }
      else
      {
        rectEnd = Vector2<int>(x, y);
      }
    }
  };

  CrawlTexture(texture, begPx, begPx + frameSize, buildRect);
  return Rect<double>((double)rectBegin.x, (double)rectBegin.y, (double)rectEnd.x, (double)rectEnd.y);
}

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
  GRenderer.Destroy();

  _gameEntities.clear();
  _networkedEntities.clear();
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

  std::function<void()> imguiWindowFunc = [this, &tracker]()
  {
    ImGui::BeginGroup();
    ImGui::Text("Update function time average %.3f ms/frame", (double)_clock.GetUpdateTime() / 1000000.0);
    ImGui::PlotLines("Update speed over time (ms/frame) - updated every 10 frames", [](void* data, int idx) { return (float)((long long*)data)[idx]/ 1000000.0f; }, tracker.GetValues(), tracker.NumValues(), 0, nullptr, FLT_MAX, FLT_MAX, ImVec2(200, 100));
    ImGui::EndGroup();
  };
  GUIController::Get().AddImguiWindowFunction("Main Debug Window", "Engine Stats", imguiWindowFunc);

  static float ts[40];
  for (int i = 0; i < 40; i++)
    ts[i] = static_cast<float>(i);

  std::function<void()> actionParameters = []()
  {
    ImGui::BeginGroup();

    ImGui::InputFloat("Walk speed", &GlobalVars::BaseWalkSpeed, 1.0f, 10.0f, 0);
    ImGui::InputFloat("Jump velocity", &GlobalVars::JumpVelocity, 1.0f, 10.0f, 0);
    ImGui::InputFloat2("Gravity force", &GlobalVars::Gravity.x, 1);
    ImGui::InputFloat2("Juggle Gravity force", &GlobalVars::JuggleGravity.x, 1);
    ImGui::InputInt("number of frames for dash", &GlobalVars::nDashFrames);
    ImGui::InputInt("Hit stop frames ON HIT", &GlobalVars::HitStopFramesOnHit);
    ImGui::InputInt("Hit stop frames ON BLOCK", &GlobalVars::HitStopFramesOnBlock);

    ImGui::Checkbox("Show hit effects", &GlobalVars::ShowHitEffects);

    if (ImGui::CollapsingHeader("Dash Function"))
    {
      // plot function for visual aid
      ImGui::PlotLines("Plateau",
      [](void* data, int idx)
      {
        return Interpolation::Plateau::F(static_cast<float*>(data)[idx], 19, 1.0f);
      },
      ts, 40, 0, nullptr, FLT_MAX, FLT_MAX, ImVec2(200, 100));

      ImGui::InputFloat("a value", &Interpolation::Plateau::a, 1.0f, 1.0f, 5);
      ImGui::InputFloat("modifier value", &Interpolation::Plateau::modifier, 0.5f, 1.0f, 5);
      ImGui::InputFloat("distribution width value", &Interpolation::Plateau::d, 0.0000001f, 0.00001f, 10);
      ImGui::InputFloat("X axis offset", &Interpolation::Plateau::xAxisOffset, 0.001f, 0.01f, 5);
    }

    ImGui::EndGroup();
  };

  std::function<void()> sceneSelect = [this]()
  {
    const char* items[] = { "Start", "Character Select", "Battle", "Results" };
    static const char* current_item = NULL;
    auto func = [this](const std::string& i)
    {
      if (i == "Start")
        RequestSceneChange(SceneType::START);
      else if (i == "Character Select")
        RequestSceneChange(SceneType::CSELECT);
      else if (i == "Battle")
        RequestSceneChange(SceneType::MATCH);
      else
        RequestSceneChange(SceneType::RESULTS);
    };
    DropDown::Show(current_item, items, 4, func);
  };

  GUIController::Get().AddImguiWindowFunction("Main Debug Window", "Dash Function parameters", actionParameters);
  GUIController::Get().AddImguiWindowFunction("Main Debug Window", "Scene Selection", sceneSelect);

  GUIController::Get().AddImguiWindowFunction("ECS Status", "Registered Components", []() {
    ImGui::Text("Components = %d", ECSGlobalStatus::NRegisteredComponents);
  });


  GUIController::Get().AddImguiWindowFunction("ECS Status", "Entity Snapshots", [this]() {

    if(ImGui::Button("Make P1 Snapshot"))
    {
      TriggerBeginningOfFrame([this]() { _p1Snapshots.push_back(_p1->CreateEntitySnapshot()); });
    }
    if (ImGui::CollapsingHeader("P1 Snapshot List"))
    {
      for (int i = 0; i < _p1Snapshots.size(); i++)
      {
        std::string btnLabel = "SNAPSHOT " + std::to_string(i + 1);
        if (ImGui::Button(btnLabel.c_str()))
        {
          TriggerBeginningOfFrame([this, i]() {_p1->LoadEntitySnapshot(_p1Snapshots[i]); });
        }
      }
    }


    if (ImGui::Button("Make P2 Snapshot"))
    {
      TriggerBeginningOfFrame([this]() {_p2Snapshots.push_back(_p2->CreateEntitySnapshot()); });
    }

    if (ImGui::CollapsingHeader("P2 Snapshot List"))
    {
      for (int i = 0; i < _p2Snapshots.size(); i++)
      {
        std::string btnLabel = "SNAPSHOT " + std::to_string(i + 1);
        if (ImGui::Button(btnLabel.c_str()))
        {
          TriggerBeginningOfFrame([this, i]() { _p2->LoadEntitySnapshot(_p2Snapshots[i]); });
        }
      }
    }

    if (ImGui::Button("Make Game State Snapshot"))
    {
      TriggerBeginningOfFrame([this]() { _gameStateSnapshots.push_back(CreateGameStateSnapshot()); });
    }

    if (ImGui::CollapsingHeader("Game State Snapshot List"))
    {
      for (int i = 0; i < _gameStateSnapshots.size(); i++)
      {
        std::string btnLabel = "SNAPSHOT " + std::to_string(i + 1);
        if (ImGui::Button(btnLabel.c_str()))
        {
          TriggerBeginningOfFrame([this, i]() { LoadGamestateSnapshot(_gameStateSnapshots[i]); });
        }
      }
    }
    
  });

  static int localPlayerIndex = 0;

#ifdef _WIN32
  GUIController::Get().AddImguiWindowFunction("GGPO", "Connect Player", [this]()
  {
    ImGui::BeginGroup();

    if (!GGPOManager::Get().InMatch())
    {
      static int localUDPPort = static_cast<int>(NetGlobals::LocalUDPPort);
      if (ImGui::InputInt("Local Port", &localUDPPort))
        NetGlobals::LocalUDPPort = static_cast<unsigned short>(localUDPPort);

      ImGui::InputInt("Frame Delay", &NetGlobals::FrameDelay);

      static char ip[128];
      ImGui::InputText("Remote Player Address", ip, 128);

      static int port;
      ImGui::InputInt("Connection Port", &port);

      if (ImGui::Button("Connect On Position 1"))
      {
        _p2->GetComponent<GameInputComponent>()->AssignHandler(InputType::NetworkCtrl);
        localPlayerIndex = 0;
        std::string_view remoteIP = ip;
        unsigned short pport = (unsigned short)port;

        GGPOPlayer players[2] = { GGPOManager::Get().CreateLocalPlayer(), GGPOManager::Get().CreateRemotePlayer(remoteIP, pport) };
        players[0].player_num = 1;
        players[1].player_num = 2;

        GGPOManager::Get().BeginSession(players);
      }

      if (ImGui::Button("Connect On Position 2"))
      {
        _p1->GetComponent<GameInputComponent>()->AssignHandler(InputType::NetworkCtrl);
        localPlayerIndex = 1;
        std::string_view remoteIP = ip;
        unsigned short pport = (unsigned short)port;

        GGPOPlayer players[2] = { GGPOManager::Get().CreateRemotePlayer(remoteIP, pport), GGPOManager::Get().CreateLocalPlayer() };
        players[0].player_num = 1;
        players[1].player_num = 2;

        GGPOManager::Get().BeginSession(players);
      }
    }
    ImGui::EndGroup();
  });

  GUIController::Get().AddImguiWindowFunction("GGPO", "Network Stats", []()
    {
      ImGui::BeginGroup();

      if (GGPOManager::Get().InMatch())
      {
        for (int i = 0; i < 2; i++)
        {
          if (i == localPlayerIndex)
            continue;

          ImGui::BeginGroup();

          GGPONetworkStats stats = GGPOManager::Get().GetPlayerStats(i);
          ImGui::Text("P%d Stats\n", (i + 1));
          ImGui::Text("Ping: %d ms", stats.network.ping);
          ImGui::Text("Frame Lag: %.1f frames", stats.network.ping ? stats.network.ping * 60.0 / 1000 : 0);
          ImGui::Text("Bandwidth: %.2f kilobytes/sec", stats.network.kbps_sent / 8.0);
          ImGui::Text("Local Frames Behind: %d frames", stats.timesync.local_frames_behind);
          ImGui::Text("Remote Frames Behind: %d frames", stats.timesync.remote_frames_behind);

          ImGui::EndGroup();
        }
      }
      ImGui::EndGroup();
    });
#endif

  CharacterEditor::Get().AddCreateNewCharacterButton();

  GUIController::Get().AddImguiWindowFunction("Assets", "Sprite Sheets", []()
  {
    ImGui::BeginGroup();
    ResourceManager::Get().gSpriteSheets.DisplayInGUI();
    ResourceManager::Get().gSpriteSheets.DisplaySaveButton(SpriteSheet::SaveLocation());
    ImGui::EndGroup();
  });

  GUIController::Get().AddImguiWindowFunction("Assets", "General Animations", []()
    {
      if (ImGui::CollapsingHeader("General Animations"))
      {
        ImGui::BeginGroup();
        GAnimArchive.EditGeneralAnimations();
        ImGui::EndGroup();
      }
    });

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
void GameManager::CheckAgainstSystems(Entity* entity)
{
  InputSystem::Check(entity);
  ApplyGravitySystem::Check(entity);
  PhysicsSystem::Check(entity);
  AnimationSystem::Check(entity);
  MoveWallSystem::Check(entity);
  AttackAnimationSystem::Check(entity);
  HitSystem::Check(entity);
  TimerSystem::Check(entity);
  FrameAdvantageSystem::Check(entity);
  SpriteDrawCallSystem::Check(entity);
  UITextDrawCallSystem::Check(entity);
  PlayerSideSystem::Check(entity);
  CutsceneSystem::Check(entity);
  CutsceneMovementSystem::Check(entity);
  CheckBattleEndSystem::Check(entity);
  UIPositionUpdateSystem::Check(entity);
  UIContainerUpdateSystem::Check(entity);
  DrawUIPrimitivesSystem::Check(entity);
  DrawUIBoxSpriteSystem::Check(entity);
  UpdateAISystem::Check(entity);
  ThrowSystem::Check(entity);
  WallPushSystem::Check(entity);
  DestroyEntitiesSystem::Check(entity);
  MenuInputSystem::Check(entity);
  UpdateMenuStateSystem::Check(entity);

  AnimationListenerSystem::Check(entity);

  // start moving stuff to aggregate systems like this
  StateTransitionAggregate::Check(entity);
  HandleUpdateAggregate::Check(entity);
  EnactAggregate::Check(entity);

  // another aggregate system
  MoveSystem::Check(entity);
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

  /*for (auto entity : _gameEntities)
  {
    Serializer<EntityID>::Serialize(stream, entity.first);
    entity.second->Serialize(stream);
  }*/

  for (const EntityID& id : _networkedEntities)
  {
    Serializer<EntityID>::Serialize(stream, id);
    _gameEntities.at(id)->Serialize(stream);
  }

  // right now, lets just copy the player entities


  //Serializer<EntityID>::Serialize(stream, _p2->GetID());
  //_p2->Serialize(stream);

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
    if (event.type == SDL_WINDOWEVENT)
    {
      if (event.window.event == SDL_WINDOWEVENT_RESIZED)
      {
        GRenderer.ProcessResizeEvent(event);
      }
    }
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
