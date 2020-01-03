#include "GameManagement.h"
#include "Timer.h"

#include "Input.h"
#include <iostream>

#include "ResourceManager.h"

#include "Components/Camera.h"
#include "Components/Sprite.h"
#include "Components/GameActor.h"
#include "Components/Physics.h"
#include "Components/Collider.h"

#include "Command.h"

#ifdef _DEBUG
//used for debugger
#include <thread>
#include <iostream>
#endif

const int m_nativeWidth = 600;
const int m_nativeHeight = 400;

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
    _loadedTextures.insert(std::make_pair(fileToLoad, Texture(_resourcePath + file)));
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

  //SDL_CreateWindowAndRenderer(ScreenWidth, ScreenHeight, 0, &_window, &_renderer);
  _window = SDL_CreateWindow(Title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    m_nativeWidth, m_nativeHeight,
    SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

  _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);

  //SDL_SetWindowTitle(_window, Title);
  
  //create game entities

  auto camera = CreateEntity<Camera>();
  camera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
  _mainCamera = camera->GetComponent<Camera>();

  Vector2<int> textureSize = ResourceManager::Get().GetTextureWidthAndHeight("spritesheets\\ryu.png");

  auto sprite = CreateEntity<Animator, Physics, GameActor, RectColliderD>();

  sprite->GetComponent<Animator>()->Init();
  sprite->GetComponent<Animator>()->RegisterAnimation("Idle", "spritesheets\\idle_and_walking.png", 6, 6, 0, 10);
  sprite->GetComponent<Animator>()->RegisterAnimation("WalkF", "spritesheets\\idle_and_walking.png", 6, 6, 10, 12);
  sprite->GetComponent<Animator>()->RegisterAnimation("WalkB", "spritesheets\\idle_and_walking.png", 6, 6, 22, 11);

  sprite->GetComponent<Animator>()->RegisterAnimation("Jumping", "spritesheets\\idle_walking_jab_jump_crouch.png", 8, 10, 41, 19);
  sprite->GetComponent<Animator>()->RegisterAnimation("Falling", "spritesheets\\idle_walking_jab_jump_crouch.png", 8, 10, 60, 13);

  sprite->GetComponent<Animator>()->RegisterAnimation("Crouching", "spritesheets\\crouching.png", 4, 5, 0, 4);
  sprite->GetComponent<Animator>()->RegisterAnimation("Crouch", "spritesheets\\crouching.png", 4, 5, 12, 5);

  sprite->GetComponent<Animator>()->RegisterAnimation("CrouchingLight", "spritesheets\\grounded_attacks.png", 8, 10, 9, 7);
  sprite->GetComponent<Animator>()->RegisterAnimation("CrouchingMedium", "spritesheets\\grounded_attacks.png", 8, 10, 16, 11);
  sprite->GetComponent<Animator>()->RegisterAnimation("CrouchingHeavy", "spritesheets\\grounded_attacks.png", 8, 10, 27, 11);
  sprite->GetComponent<Animator>()->RegisterAnimation("StandingLight", "spritesheets\\grounded_attacks.png", 8, 10, 40, 6);
  sprite->GetComponent<Animator>()->RegisterAnimation("StandingMedium", "spritesheets\\grounded_attacks.png", 8, 10, 46, 9);
  sprite->GetComponent<Animator>()->RegisterAnimation("StandingHeavy", "spritesheets\\grounded_attacks.png", 8, 10, 55, 13);
  sprite->GetComponent<Animator>()->RegisterAnimation("JumpingLight", "spritesheets\\jlp.png", 4, 4, 0, 14);
  sprite->GetComponent<Animator>()->RegisterAnimation("JumpingMedium", "spritesheets\\jlp.png", 4, 4, 0, 14);
  sprite->GetComponent<Animator>()->RegisterAnimation("JumpingHeavy", "spritesheets\\jlp.png", 4, 4, 0, 14);

  sprite->GetComponent<Animator>()->Play("Idle", true);

  sprite->GetComponent<RectColliderD>()->Init(Vector2<double>(0.0, 0.0),
    Vector2<double>(static_cast<double>(textureSize.x), static_cast<double>(textureSize.y)));
  sprite->GetComponent<RectColliderD>()->SetStatic(false);
  _player = sprite->GetComponent<GameActor>();

  sprite->SetScale(Vector2<float>(1.4f, 1.7f));
  
  /*auto staticBoy = CreateEntity<Animator, RectColliderD>();
  float startPosX = staticBoy->transform.position.x = 200.0f;
  float startPosY = staticBoy->transform.position.y = 200.0f;

  staticBoy->GetComponent<Animator>()->Init();
  staticBoy->GetComponent<Animator>()->RegisterAnimation("Idle", "spritesheets\\idle.png", 3, 4, 0, 10);
  staticBoy->GetComponent<Animator>()->Play("Idle", true);
  staticBoy->GetComponent<RectColliderD>()->Init(Vector2<double>(startPosX, startPosY), Vector2<double>(startPosX + textureSize.x, startPosY + textureSize.y));
  staticBoy->GetComponent<RectColliderD>()->SetStatic(true);*/

  auto bottomBorder = CreateEntity<Sprite, RectColliderD>();
  bottomBorder->transform.position.x = 0.0;
  bottomBorder->transform.position.y = m_nativeHeight - 40;
  bottomBorder->GetComponent<Sprite>()->Init("spritesheets\\ryu.png");
  bottomBorder->GetComponent<RectColliderD>()->Init(Vector2<double>(0, m_nativeHeight - 40), Vector2<double>(m_nativeWidth, m_nativeHeight + 5000.0f));
  bottomBorder->GetComponent<RectColliderD>()->SetStatic(true);


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

  _player.reset();
  _mainCamera.reset();
  _gameEntities.clear();
}

//______________________________________________________________________________
void GameManager::BeginGameLoop()
{
  SDL_Event event;
  Timer clock;
  //start the timer
  clock.Start();

  //initialize the functions
  //FrameFunction input = std::bind(&GameManager::UpdateInput, this);
  UpdateFunction update = std::bind(&GameManager::Update, this, std::placeholders::_1);
  //FrameFunction draw = std::bind(&GameManager::Draw, this);

  bool programRunning = true;
#ifdef _DEBUG
  std::thread debugger([this, &programRunning]()
  {
    while(programRunning)
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

  for (;;)
  {
    //! Check for quit
    if (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT)
      {
        return;
      }
      if (event.type == SDL_WINDOWEVENT)
      {
        if (event.window.event == SDL_WINDOWEVENT_RESIZED)
        {
          Vector2<int> newWindowSize(event.window.data1, event.window.data2);
          widthToScreenWidth = static_cast<double>(newWindowSize.x) / static_cast<double>(m_nativeWidth);
          heightToScreenHeight = static_cast<double>(newWindowSize.y) / static_cast<double>(m_nativeHeight);
          SDL_RenderSetScale(_renderer, static_cast<float>(widthToScreenWidth), static_cast<float>(heightToScreenHeight));
        }
      }
    }

    //! Do pre-input set up stuff
    for (auto sprite : ComponentManager<Sprite>::Get().All())
      sprite->OnFrameBegin();
    for (auto animator : ComponentManager<Animator>::Get().All())
      animator->OnFrameBegin();


    //! Collect inputs from controllers (this means AI controllers as well as Player controllers)
    UpdateInput(&event);

    std::lock_guard lock(_debugMutex);
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
void GameManager::Update(float deltaTime)
{
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
}

//______________________________________________________________________________
void GameManager::UpdateInput(SDL_Event* event)
{
  //update keys pressed here
  auto command = std::unique_ptr<ICommand>(_playerInput->HandleInput(event));

  if (command)
    command->Execute(_player.get());
}

//______________________________________________________________________________
void GameManager::Draw()
{
  //clear last frame graphics
  SDL_RenderClear(_renderer);

  // resource manager draw here?
  ResourceManager::Get().BlitSprites();

  //present this frame
  SDL_RenderPresent(_renderer);
}

//______________________________________________________________________________
GameManager::GameManager() : _initialized(false)
{
  _playerInput = std::unique_ptr<IInputHandler>(new KeyboardInputHandler());
}
