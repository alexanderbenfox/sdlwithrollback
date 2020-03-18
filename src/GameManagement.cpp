#include "GameManagement.h"
#include "Timer.h"

#include <iostream>

#include "ResourceManager.h"

#include "Components/Camera.h"
#include "Components/Animator.h"
#include "Components/GameActor.h"
#include "Components/Rigidbody.h"
#include "Components/Collider.h"
#include "Components/Input.h"

#include "Systems/Physics.h"
#include "Systems/AnimationSystem.h"
#include "Systems/MoveSystem.h"
#include "Systems/InputSystem.h"
#include "Systems/HitSystem.h"
#include "Systems/TimerSystem.h"

#include "Systems/Physics.h"

#ifdef _DEBUG
//used for debugger
#include <thread>
#include <iostream>
#endif

static double widthToScreenWidth = 1.0;
static double heightToScreenHeight = 1.0f;

const char* Title = "Game Title";

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
Texture& ResourceManager::GetTexture(const std::string& file)
{
  auto fileToLoad = file;

#ifndef _WIN32
  auto split = StringUtils::Split(file, '\\');
  if(split.size() > 1)
    fileToLoad = StringUtils::Connect(split.begin(), split.end(), '/');
#endif

  if (_loadedTextures.find(fileToLoad) == _loadedTextures.end())
  {
    _loadedTextures.insert(std::make_pair(fileToLoad, Texture(_resourcePath + fileToLoad)));
  }
  _loadedTextures[fileToLoad].Load();
  return _loadedTextures[fileToLoad];
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
  SDL_QueryTexture(GetTexture(fileToQuery).Get(), nullptr, nullptr, &width, &height);
  return Vector2<int>(width, height);
}

//______________________________________________________________________________
void ResourceManager::RegisterBlitOp()
{
  _registeredSprites.push_back(BlitOperation());
  _registeredSprites.back().valid = false;
}

//______________________________________________________________________________
void ResourceManager::BlitSprites()
{
  auto blit = [](BlitOperation* operation)
  {
    if (!operation->valid) return;

    GameManager::Get().GetMainCamera()->ConvScreenSpace(operation);
    if (GameManager::Get().GetMainCamera()->EntityInDisplay(operation))
    {
      int w, h;
      auto srcTexture = operation->_textureResource->Get();
      float rotation = 0;
      
      try
      {
        if (SDL_QueryTexture(operation->_textureResource->Get(), NULL, NULL, &w, &h) == 0)
        {
          SDL_RenderCopyEx(GameManager::Get().GetRenderer(), srcTexture,
            &operation->_textureRect, &operation->_displayRect, rotation, nullptr, operation->_flip);
        }
      }
      catch (std::exception &e)
      {
        std::cout << "I guess this texture isn't valid??" << "\nCaught exception: " << e.what() << "\n";
      }
    }
  };

  for (auto& sprite : _registeredSprites)
  {
    blit(&sprite);
    // deregister sprite from list
    //sprite.valid = false;
  }

  //reset available ops for next frame
  opIndex = 0;
}

//______________________________________________________________________________
void ResourceManager::CrawlTexture(Texture& texture, Vector2<int> begin, Vector2<int> end, std::function<void(int, int, Uint32)> callback)
{
  struct SDLTextureInfo
  {
    SDLTextureInfo(SDL_Texture* texture) : texture(texture)
    {
      SDL_LockTexture(texture, nullptr, &pixels, &pitch);
    }
    ~SDLTextureInfo()
    {
      SDL_UnlockTexture(texture);
    }

    void* pixels;
    int pitch;
    SDL_Texture* texture;
  };

  auto& textureData = texture.GetInfo();
  // Get the window format
  Uint32 windowFormat = SDL_GetWindowPixelFormat(GameManager::Get().GetWindow());
  std::shared_ptr<SDL_PixelFormat> format = std::shared_ptr<SDL_PixelFormat>(SDL_AllocFormat(windowFormat), SDL_FreeFormat);

  // Get the pixel data
  Uint32* upixels;

#ifdef _WIN32
  auto textureInfo = SDLTextureInfo(texture.Get());
  unsigned char* px = (unsigned char*)textureInfo.pixels;
  upixels = (Uint32*)textureInfo.pixels;
  Uint32 transparent = SDL_MapRGBA(format.get(), px[0], px[1], px[2], 0x00);
#else
  upixels = (Uint32*)texture.GetInfo().pixels.get();
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
Rect<double> ResourceManager::FindRect(Texture& texture, Vector2<int> frameSize, Vector2<int> begPx)
{
  Uint32 windowFormat = SDL_GetWindowPixelFormat(GameManager::Get().GetWindow());
  std::shared_ptr<SDL_PixelFormat> format = std::shared_ptr<SDL_PixelFormat>(SDL_AllocFormat(windowFormat), SDL_FreeFormat);

  Uint32 transparent;
#ifdef _WIN32
  auto textureInfo = SDLTextureInfo(texture.Get());
  unsigned char* px = (unsigned char*)textureInfo.pixels;
  Uint32* upixels = (Uint32*)textureInfo.pixels;
  transparent = SDL_MapRGBA(format.get(), px[0], px[1], px[2], 0x00);
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
  SDL_Init(SDL_INIT_EVERYTHING);
  TTF_Init();

  _window = SDL_CreateWindow(Title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    m_nativeWidth, m_nativeHeight,
    SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

  _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
  SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);

  auto bottomBorder = CreateEntity<Transform, SpriteRenderer, RectColliderD>();
  bottomBorder->GetComponent<Transform>()->position.x = 0.0;
  bottomBorder->GetComponent<Transform>()->position.y = m_nativeHeight - 40;
  bottomBorder->GetComponent<SpriteRenderer>()->Init("spritesheets\\ryu.png", false);
  bottomBorder->GetComponent<RectColliderD>()->Init(Vector2<double>(0, m_nativeHeight - 40), Vector2<double>(m_nativeWidth, m_nativeHeight + 5000.0f));
  bottomBorder->GetComponent<RectColliderD>()->SetStatic(true);

  auto leftBorder = CreateEntity<Transform, SpriteRenderer, RectColliderD>();
  leftBorder->GetComponent<Transform>()->position.x = -200;
  leftBorder->GetComponent<Transform>()->position.y = 0;
  leftBorder->GetComponent<SpriteRenderer>()->Init("spritesheets\\ryu.png", false);
  leftBorder->GetComponent<RectColliderD>()->Init(Vector2<double>(-200, 0), Vector2<double>(0, m_nativeHeight));
  leftBorder->GetComponent<RectColliderD>()->SetStatic(true);

  auto rightBorder = CreateEntity<Transform, SpriteRenderer, RectColliderD>();
  rightBorder->GetComponent<Transform>()->position.x = m_nativeWidth;
  rightBorder->GetComponent<Transform>()->position.y = 0;
  rightBorder->GetComponent<SpriteRenderer>()->Init("spritesheets\\ryu.png", false);
  rightBorder->GetComponent<RectColliderD>()->Init(Vector2<double>(m_nativeWidth, 0), Vector2<double>(m_nativeWidth + 200, m_nativeHeight));
  rightBorder->GetComponent<RectColliderD>()->SetStatic(true);


  auto p1 = EntityCreation::CreateLocalPlayer(0);
  auto p2 = EntityCreation::CreateLocalPlayer(150);

  auto kb2 = p2->GetComponent<KeyboardInputHandler>();
  kb2->SetKey(SDLK_UP, InputState::UP);
  kb2->SetKey(SDLK_DOWN, InputState::DOWN);
  kb2->SetKey(SDLK_RIGHT, InputState::RIGHT);
  kb2->SetKey(SDLK_LEFT, InputState::LEFT);
  kb2->SetKey(SDLK_j, InputState::BTN1);
  kb2->SetKey(SDLK_k, InputState::BTN2);
  kb2->SetKey(SDLK_l, InputState::BTN3);

  _camera = EntityCreation::CreateCamera();

  //_gameState = std::unique_ptr<IGameState>(new LocalMatch(kb1, kb2));
}

//______________________________________________________________________________
void GameManager::Destroy()
{
  SDL_DestroyRenderer(_renderer);
  SDL_DestroyWindow(_window);

  _renderer = nullptr;
  _window = nullptr;

  SDL_Quit();
  TTF_Quit();

  _gameEntities.clear();
}

//______________________________________________________________________________
void GameManager::BeginGameLoop()
{
  Timer clock;
  //start the timer
  clock.Start();

  //initialize the functions
  //FrameFunction input = std::bind(&GameManager::UpdateInput, this);
  UpdateFunction update = std::bind(&GameManager::Update, this, std::placeholders::_1);
  //FrameFunction draw = std::bind(&GameManager::Draw, this);

  bool programRunning = true;
  std::thread debuggerThread;
  RunScripter(debuggerThread, programRunning);

  for (;;)
  {
    //! Collect inputs from controllers (this means AI controllers as well as Player controllers)
    //UpdateInput();
    //InputSystem::DoTick(0);

    std::lock_guard<std::mutex> lock(_debugMutex);
    //! Update all components
    clock.Update(update);

    // do once per frame system calls
    AnimationSystem::PostUpdate();

    //! Finally render the scene
    Draw();

    //! Do post-frame resolution stuff
    for (auto actor : ComponentManager<GameActor>::Get().All())
      actor->OnFrameEnd();
  }

  programRunning = false;
}

//______________________________________________________________________________
Camera* GameManager::GetMainCamera()
{
  //return _gameState->GetCamera();
  return _camera.get();
}

//______________________________________________________________________________
void GameManager::CheckAgainstSystems(Entity* entity)
{
  PhysicsSystem::Check(entity);
  AnimationSystem::Check(entity);
  MoveSystemRect::Check(entity);
  MoveSystemCamera::Check(entity);
  InputSystem::Check(entity);
  HitSystem::Check(entity);
  TimerSystem::Check(entity);
}

//______________________________________________________________________________
void GameManager::Update(float deltaTime)
{
  UpdateInput();
  TimerSystem::DoTick(deltaTime);
  HitSystem::DoTick(deltaTime);
  InputSystem::DoTick(deltaTime);
  // resolve collisions
  PhysicsSystem::DoTick(deltaTime);
  // update the location of the colliders
  MoveSystem::DoTick(deltaTime);
  // update rendered components last
  AnimationSystem::DoTick(deltaTime);
}

//______________________________________________________________________________
void GameManager::UpdateInput()
{
  // Process local input first
  //! Check for quit
  if (SDL_PollEvent(&_localInput)) {
    if (_localInput.type == SDL_QUIT)
    {
      return;
    }
    if (_localInput.type == SDL_WINDOWEVENT)
    {
      if (_localInput.window.event == SDL_WINDOWEVENT_RESIZED)
      {
        Vector2<int> newWindowSize(_localInput.window.data1, _localInput.window.data2);
        widthToScreenWidth = static_cast<double>(newWindowSize.x) / static_cast<double>(m_nativeWidth);
        heightToScreenHeight = static_cast<double>(newWindowSize.y) / static_cast<double>(m_nativeHeight);
        SDL_RenderSetScale(_renderer, static_cast<float>(widthToScreenWidth), static_cast<float>(heightToScreenHeight));
      }
    }
  }
}

//______________________________________________________________________________
void GameManager::Draw()
{
  //clear last frame graphics
  SDL_RenderClear(_renderer);

  // resource manager draw here?
  ResourceManager::Get().BlitSprites();

  // draw debug rects
  ComponentManager<Hurtbox>::Get().Draw();
  ComponentManager<Hitbox>::Get().Draw();

  //present this frame
  SDL_RenderPresent(_renderer);
}

//______________________________________________________________________________
void GameManager::RunScripter(std::thread& t, bool& programRunning)
{
#ifdef _DEBUG
  t = std::thread([this, &programRunning]()
  {
    while (programRunning)
    {
      std::cout << "Example: 'entity0 transform set scale x 0.5'" << "\n";
      std::string command;
      std::getline(std::cin, command);

      auto split = StringUtils::Split(command, ' ');
      if (split.size() <= 1) continue;
      auto id = split[0];

      for (auto& entity : _gameEntities)
      {
        if (entity->GetIdentifier() == id)
        {
          std::lock_guard lock(_debugMutex);
          entity->ParseCommand(StringUtils::Connect(split.begin() + 1, split.end(), ' '));
          break;
        }
      }

      if (UniversalPhysicsSettings::Get().GetIdentifier() == id)
      {
        UniversalPhysicsSettings::Get().ParseCommand(StringUtils::Connect(split.begin() + 1, split.end(), ' '));
      }
    }
  });
#endif
}

//______________________________________________________________________________
GameManager::GameManager() : _initialized(false) {}
