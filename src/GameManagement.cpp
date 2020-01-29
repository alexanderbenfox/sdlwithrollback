#include "GameManagement.h"
#include "Timer.h"

#include <iostream>

#include "ResourceManager.h"

#include "Components/Camera.h"
#include "Components/Animator.h"
#include "Components/GameActor.h"
#include "Components/Physics.h"
#include "Components/Collider.h"

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
void GameManager::Initialize()
{
  SDL_Init(SDL_INIT_EVERYTHING);
  TTF_Init();

  _window = SDL_CreateWindow(Title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    m_nativeWidth, m_nativeHeight,
    SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

  _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
  SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);

  auto bottomBorder = CreateEntity<Sprite, RectColliderD>();
  bottomBorder->transform.position.x = 0.0;
  bottomBorder->transform.position.y = m_nativeHeight - 40;
  bottomBorder->GetComponent<Sprite>()->Init("spritesheets\\ryu.png", false);
  bottomBorder->GetComponent<RectColliderD>()->Init(Vector2<double>(0, m_nativeHeight - 40), Vector2<double>(m_nativeWidth, m_nativeHeight + 5000.0f));
  bottomBorder->GetComponent<RectColliderD>()->SetStatic(true);

  auto leftBorder = CreateEntity<Sprite, RectColliderD>();
  leftBorder->transform.position.x = -200;
  leftBorder->transform.position.y = 0;
  leftBorder->GetComponent<Sprite>()->Init("spritesheets\\ryu.png", false);
  leftBorder->GetComponent<RectColliderD>()->Init(Vector2<double>(-200, 0), Vector2<double>(0, m_nativeHeight));
  leftBorder->GetComponent<RectColliderD>()->SetStatic(true);

  auto rightBorder = CreateEntity<Sprite, RectColliderD>();
  rightBorder->transform.position.x = m_nativeWidth;
  rightBorder->transform.position.y = 0;
  rightBorder->GetComponent<Sprite>()->Init("spritesheets\\ryu.png", false);
  rightBorder->GetComponent<RectColliderD>()->Init(Vector2<double>(m_nativeWidth, 0), Vector2<double>(m_nativeWidth + 200, m_nativeHeight));
  rightBorder->GetComponent<RectColliderD>()->SetStatic(true);

  KeyboardInputHandler* kb1 = new KeyboardInputHandler();
  KeyboardInputHandler* kb2 = new KeyboardInputHandler();
  kb2->SetKey(SDLK_UP, InputState::UP);
  kb2->SetKey(SDLK_DOWN, InputState::DOWN);
  kb2->SetKey(SDLK_RIGHT, InputState::RIGHT);
  kb2->SetKey(SDLK_LEFT, InputState::LEFT);

  _gameState = std::unique_ptr<IGameState>(new LocalMatch(kb1, kb2));
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
    //! Do pre-input set up stuff
    for (auto sprite : ComponentManager<Sprite>::Get().All())
      sprite->OnFrameBegin();
    for (auto animator : ComponentManager<Animator>::Get().All())
      animator->OnFrameBegin();

    //! Collect inputs from controllers (this means AI controllers as well as Player controllers)
    UpdateInput();

    std::lock_guard<std::mutex> lock(_debugMutex);
    //! Update all components
    clock.Update(update);

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
  return _gameState->GetCamera();
}

//______________________________________________________________________________
void GameManager::Update(float deltaTime)
{
  //====PREUPDATE=====//
  ComponentManager<Physics>::Get().PreUpdate();

  // update acting units' state machine
  ComponentManager<GameActor>::Get().Update(deltaTime);
  // resolve collisions
  ComponentManager<Physics>::Get().Update(deltaTime);
  // update the location of the colliders
  ComponentManager<RectColliderD>::Get().Update(deltaTime);

  // update rendered components last
  ComponentManager<Sprite>::Get().Update(deltaTime);
  // update animation state
  ComponentManager<Animator>::Get().Update(deltaTime);

  //====POSTUPDATE=====//
  ComponentManager<Physics>::Get().PostUpdate();
  // update the location of the colliders (again)
  ComponentManager<RectColliderD>::Get().Update(deltaTime);
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
  _gameState->ProcessRawInputs(&_localInput);
}

//______________________________________________________________________________
void GameManager::Draw()
{
  //clear last frame graphics
  SDL_RenderClear(_renderer);

  // resource manager draw here?
  ResourceManager::Get().BlitSprites();

  // draw debug rects
  ComponentManager<RectColliderD>::Get().Draw();

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
