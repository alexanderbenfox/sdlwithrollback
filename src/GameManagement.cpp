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

#include "Systems/Physics.h"
#include "DebugGUI/GUIController.h"

#include "GameState/Scene.h"

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
    if (r != 0)
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

  //! initialize the scene
  _currentScene = std::make_unique<BattleScene>();
  _currentScene->Init();

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

  int frameCount = 0;
  for (;;)
  {
    if (frameCount == 0)
    {
      tracker.Add(_clock.GetUpdateTime());
    }

    //! Update all components and coroutines
    _clock.Update(update);
    //! update debug gui
    GUIController::Get().MainLoop(_localInput);

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
  GamepadInputSystem::Check(entity);
  PhysicsSystem::Check(entity);
  AnimationSystem::Check(entity);
  MoveSystemRect::Check(entity);
  MoveSystemCamera::Check(entity);
  AttackAnimationSystem::Check(entity);
  HitSystem::Check(entity);
  TimerSystem::Check(entity);
  FrameAdvantageSystem::Check(entity);
  SpriteDrawCallSystem::Check(entity);
  UITextDrawCallSystem::Check(entity);
  PlayerSideSystem::Check(entity);
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
void GameManager::Update(float deltaTime)
{
  UpdateInput();
  _currentScene->Update(deltaTime);
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
