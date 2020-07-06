#include "GameManagement.h"

#include <iostream>

#include "ResourceManager.h"

#include "Systems/Physics.h"
#include "Systems/AnimationSystem.h"
#include "Systems/DrawCallSystems.h"
#include "Systems/MoveSystem.h"
#include "Systems/InputSystem.h"
#include "Systems/HitSystem.h"
#include "Systems/TimerSystem.h"
#include "Systems/SceneSystems.h"
#include "Systems/CutsceneSystem.h"
#include "Systems/CheckBattleEndSystem.h"
#include "Systems/UISystem.h"

#include "Systems/Physics.h"
#include "DebugGUI/GUIController.h"

#include "GameState/Scene.h"

#include "StateMachine/ActionUtil.h"

#ifdef _DEBUG
//used for debugger
#include <thread>
#include <iostream>
#endif

int ConstComponentIDGenerator::ID = 0;

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
    _loadedTexts.insert(std::make_pair(text, TextResource(font, text, SDL_Color{255, 255, 255, SDL_ALPHA_OPAQUE})));
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
    auto& font = GetAsset<TTF_Font>(fontFile);
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
  unsigned char* px = textureData.pixels.get();
  upixels = (Uint32*)px;
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
  unsigned char* px = texture.GetInfo().pixels.get();
  Uint32* upixels = (Uint32*)px;
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

  //! initialize our player controllable entities
  _p1 = CreateEntity<GameInputComponent>();
  _p2 = CreateEntity<GameInputComponent>();

  //! Initialize them with keyboard handlers
  _p1->GetComponent<GameInputComponent>()->AssignHandler(InputType::Keyboard);
  _p2->GetComponent<GameInputComponent>()->AssignHandler(InputType::Keyboard);

  //! initialize the scene
  ChangeScene(SceneType::START);

  _initialized = true;
}

//______________________________________________________________________________
void GameManager::Destroy()
{
  GRenderer.Destroy();

  _gameEntities.clear();
}

//______________________________________________________________________________
void GameManager::BeginGameLoop()
{
  //start the timer
  _clock.Start();

  //initialize the functions
  UpdateFunction update = std::bind(&GameManager::Update, this, std::placeholders::_1);

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
  GUIController::Get().AddImguiWindowFunction("Engine Stats", imguiWindowFunc);

  static float ts[40];
  for (int i = 0; i < 40; i++)
    ts[i] = static_cast<float>(i);

  std::function<void()> actionParameters = []()
  {
    ImGui::BeginGroup();
    ImGui::InputFloat("a value", &Interpolation::Plateau::a, 1.0f, 1.0f, 5);
    ImGui::InputFloat("modifier value", &Interpolation::Plateau::modifier, 0.5f, 1.0f, 5);
    ImGui::InputFloat("distribution width value", &Interpolation::Plateau::d, 0.0000001f, 0.00001f, 10);
    ImGui::InputFloat("X axis offset", &Interpolation::Plateau::xAxisOffset, 0.001f, 0.01f, 5);
    ImGui::InputInt("number of frames for dash", &ActionParams::nDashFrames);
    ImGui::InputFloat("Walk speed", &ActionParams::baseWalkSpeed, 1.0f, 10.0f, 0);
    
    // plot function for visual aid
    ImGui::PlotLines("Plateau",
      [](void* data, int idx)
      {
        return Interpolation::Plateau::F(static_cast<float*>(data)[idx], 19, 1.0f);
      }, ts, 40, 0, nullptr, FLT_MAX, FLT_MAX, ImVec2(200, 100));
    ImGui::EndGroup();
  };

  std::function<void()> sceneSelect = [this]()
  {
    const char* items[] = { "Start", "Character Select", "Battle", "Results" };
    static const char* current_item = NULL;
    auto func = [this](const std::string& i)
    {
      if (i == "Start")
        ChangeScene(SceneType::START);
      else if (i == "Character Select")
        ChangeScene(SceneType::CSELECT);
      else if (i == "Battle")
        ChangeScene(SceneType::BATTLE);
      else
        ChangeScene(SceneType::RESULTS);
    };
    DropDown::Show(current_item, items, 4, func);
  };

  GUIController::Get().AddImguiWindowFunction("Dash Function parameters", actionParameters);
  GUIController::Get().AddImguiWindowFunction("Scene Selection", sceneSelect);

  int frameCount = 0;
  for (;;)
  {
    if (frameCount == 0)
      tracker.Add(_clock.GetUpdateTime());

    //! Update all components and coroutines
    _clock.Update(update);
    //! Do post update (update gui and change scene)
    PostUpdate();
    //! Finally render the scene
    Draw();

    if (_localInput.type == SDL_QUIT)
      break;

    frameCount = (++frameCount) % 10;
  }

  GUIController::Get().CleanUp();
}

//______________________________________________________________________________
Camera* GameManager::GetMainCamera()
{
  if (_currentScene)
    return _currentScene->GetCamera();
  return nullptr;
}

//______________________________________________________________________________
void GameManager::CheckAgainstSystems(Entity* entity)
{
  InputSystem::Check(entity);
  PhysicsSystem::Check(entity);
  AnimationSystem::Check(entity);
  MoveSystemPhysCollider::Check(entity);
  MoveSystemHurtbox::Check(entity);
  MoveSystemCamera::Check(entity);
  MoveSystemHitbox::Check(entity);
  AttackAnimationSystem::Check(entity);
  HitSystem::Check(entity);
  TimerSystem::Check(entity);
  FrameAdvantageSystem::Check(entity);
  SpriteDrawCallSystem::Check(entity);
  UITextDrawCallSystem::Check(entity);
  PlayerSideSystem::Check(entity);
  StartSceneInputSystem::Check(entity);
  CharacterSelectInputSystem::Check(entity);
  ResultsSceneSystem::Check(entity);
  CutsceneSystem::Check(entity);
  CutsceneMovementSystem::Check(entity);
  CheckBattleEndSystem::Check(entity);
  UIPositionUpdateSystem::Check(entity);
  UIContainerUpdateSystem::Check(entity);
  DrawUIPrimitivesSystem::Check(entity);
}

//______________________________________________________________________________
void GameManager::ActivateHitStop(int frames)
{
  _clock.PauseForTime(static_cast<float>(frames) * secPerFrame);
}

//______________________________________________________________________________
void GameManager::DebugDraws()
{
  // draw items in debug layer over top of the drawn scene
  ComponentManager<Hurtbox>::Get().Draw();
  ComponentManager<Hitbox>::Get().Draw();
}

//______________________________________________________________________________
void GameManager::DestroyEntity(std::shared_ptr<Entity> entity)
{
  auto it = std::find(_gameEntities.begin(), _gameEntities.end(), entity);
  if(it != _gameEntities.end())
  {
    (*it)->RemoveAllComponents();
    _gameEntities.erase(it, it + 1);
  }
}

//______________________________________________________________________________
void GameManager::RequestSceneChange(SceneType newSceneType)
{
  _sceneChangeRequested = true;
  _currentSceneType = newSceneType;
}

//______________________________________________________________________________
void GameManager::ChangeScene(SceneType scene)
{
  _currentScene.reset();

  for (auto& func : _onSceneChangeFunctionQueue)
    func();
  _onSceneChangeFunctionQueue.clear();

  _currentScene = std::unique_ptr<IScene>(SceneHelper::CreateScene(scene));
  _currentScene->Init(_p1, _p2);
  _currentSceneType = scene;
}

//______________________________________________________________________________
void GameManager::Update(float deltaTime)
{
  UpdateInput();
  _currentScene->Update(deltaTime);
}

//______________________________________________________________________________
void GameManager::PostUpdate()
{
  //! update debug gui
  GUIController::Get().MainLoop(_localInput);
  // defer scene change until end of update loops
  if(_sceneChangeRequested)
  {
    ChangeScene(_currentSceneType);
    _sceneChangeRequested = false;
  }

  if (!_endOfFrameQueue.empty())
  {
    for (auto& func : _endOfFrameQueue)
      func();
    _endOfFrameQueue.clear();
  }
}

//______________________________________________________________________________
void GameManager::UpdateInput()
{
  // Check for quit or resize and update input object
  if (SDL_PollEvent(&_localInput)) {
    if (_localInput.type == SDL_WINDOWEVENT)
    {
      if (_localInput.window.event == SDL_WINDOWEVENT_RESIZED)
      {
        RenderManager<RenderType>::Get().ProcessResizeEvent(_localInput);
      }
    }
  }
}

//______________________________________________________________________________
void GameManager::Draw()
{
  // send draw calls to the renderer
  SpriteDrawCallSystem::PostUpdate();
  UITextDrawCallSystem::PostUpdate();
  DrawUIPrimitivesSystem::PostUpdate();

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
GameManager::GameManager() : _initialized(false) {}
