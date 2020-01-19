#include "GameState/GameState.h"
#include "Command.h"

#include "GameManagement.h"
#include "Components/Sprite.h"
#include "Components/GameActor.h"
#include "Components/Physics.h"
#include "Components/Camera.h"

void LocalMatch::ProcessInputs(SDL_Event* localInput)
{
  //update keys pressed here
  auto command_P1 = std::unique_ptr<ICommand>(new InGameCommand(_player1.input->HandleInput(localInput)));
  auto command_P2 = std::unique_ptr<ICommand>(new InGameCommand(_player2.input->HandleInput(localInput)));

  // advance game state?
  _player1.ExecuteInput(command_P1.get());
  _player2.ExecuteInput(command_P2.get());
}

LocalPlayer EntityCreation::CreateLocalPlayer(IInputHandler<SDL_Event>* input, float xOffset)
{
  Vector2<int> textureSize = ResourceManager::Get().GetTextureWidthAndHeight("spritesheets\\ryu.png");

  auto player = GameManager::Get().CreateEntity<Animator, Physics, GameActor, RectColliderD>();

  player->GetComponent<Animator>()->Init();
  player->GetComponent<Animator>()->RegisterAnimation("Idle", "spritesheets\\idle_and_walking.png", 6, 6, 0, 10);
  player->GetComponent<Animator>()->RegisterAnimation("WalkF", "spritesheets\\idle_and_walking.png", 6, 6, 10, 12);
  player->GetComponent<Animator>()->RegisterAnimation("WalkB", "spritesheets\\idle_and_walking.png", 6, 6, 22, 11);

  player->GetComponent<Animator>()->RegisterAnimation("Jumping", "spritesheets\\idle_walking_jab_jump_crouch.png", 8, 10, 41, 19);
  player->GetComponent<Animator>()->RegisterAnimation("Falling", "spritesheets\\idle_walking_jab_jump_crouch.png", 8, 10, 60, 13);

  player->GetComponent<Animator>()->RegisterAnimation("Crouching", "spritesheets\\crouching.png", 4, 5, 0, 4);
  player->GetComponent<Animator>()->RegisterAnimation("Crouch", "spritesheets\\crouching.png", 4, 5, 12, 5);

  player->GetComponent<Animator>()->RegisterAnimation("CrouchingLight", "spritesheets\\grounded_attacks.png", 8, 10, 9, 7);
  player->GetComponent<Animator>()->RegisterAnimation("CrouchingMedium", "spritesheets\\grounded_attacks.png", 8, 10, 16, 11);
  player->GetComponent<Animator>()->RegisterAnimation("CrouchingHeavy", "spritesheets\\grounded_attacks.png", 8, 10, 27, 11);
  player->GetComponent<Animator>()->RegisterAnimation("StandingLight", "spritesheets\\grounded_attacks.png", 8, 10, 40, 6);
  player->GetComponent<Animator>()->RegisterAnimation("StandingMedium", "spritesheets\\grounded_attacks.png", 8, 10, 46, 9);
  player->GetComponent<Animator>()->RegisterAnimation("StandingHeavy", "spritesheets\\grounded_attacks.png", 8, 10, 55, 13);
  player->GetComponent<Animator>()->RegisterAnimation("JumpingLight", "spritesheets\\jlp.png", 4, 4, 0, 14);
  player->GetComponent<Animator>()->RegisterAnimation("JumpingMedium", "spritesheets\\jlp.png", 4, 4, 0, 14);
  player->GetComponent<Animator>()->RegisterAnimation("JumpingHeavy", "spritesheets\\jlp.png", 4, 4, 0, 14);

  player->GetComponent<Animator>()->Play("Idle", true);

  player->GetComponent<RectColliderD>()->Init(Vector2<double>(xOffset, 0.0),
    Vector2<double>(xOffset + static_cast<double>(textureSize.x), static_cast<double>(textureSize.y)));
  player->GetComponent<RectColliderD>()->SetStatic(false);

  player->SetScale(Vector2<float>(1.4f, 1.7f));
  player->transform.position.x = xOffset;

  player->GetComponent<RectColliderD>()->Update(0);

  return LocalPlayer(input, player);
}

#ifdef _WIN32
NetworkPlayer EntityCreation::CreateNetworkPlayer(IInputHandler<GGPOInput>* input, float xOffset)
{
  Vector2<int> textureSize = ResourceManager::Get().GetTextureWidthAndHeight("spritesheets\\ryu.png");

  auto player = GameManager::Get().CreateEntity<Animator, Physics, GameActor, RectColliderD>();

  player->GetComponent<Animator>()->Init();
  player->GetComponent<Animator>()->RegisterAnimation("Idle", "spritesheets\\idle_and_walking.png", 6, 6, 0, 10);
  player->GetComponent<Animator>()->RegisterAnimation("WalkF", "spritesheets\\idle_and_walking.png", 6, 6, 10, 12);
  player->GetComponent<Animator>()->RegisterAnimation("WalkB", "spritesheets\\idle_and_walking.png", 6, 6, 22, 11);

  player->GetComponent<Animator>()->RegisterAnimation("Jumping", "spritesheets\\idle_walking_jab_jump_crouch.png", 8, 10, 41, 19);
  player->GetComponent<Animator>()->RegisterAnimation("Falling", "spritesheets\\idle_walking_jab_jump_crouch.png", 8, 10, 60, 13);

  player->GetComponent<Animator>()->RegisterAnimation("Crouching", "spritesheets\\crouching.png", 4, 5, 0, 4);
  player->GetComponent<Animator>()->RegisterAnimation("Crouch", "spritesheets\\crouching.png", 4, 5, 12, 5);

  player->GetComponent<Animator>()->RegisterAnimation("CrouchingLight", "spritesheets\\grounded_attacks.png", 8, 10, 9, 7);
  player->GetComponent<Animator>()->RegisterAnimation("CrouchingMedium", "spritesheets\\grounded_attacks.png", 8, 10, 16, 11);
  player->GetComponent<Animator>()->RegisterAnimation("CrouchingHeavy", "spritesheets\\grounded_attacks.png", 8, 10, 27, 11);
  player->GetComponent<Animator>()->RegisterAnimation("StandingLight", "spritesheets\\grounded_attacks.png", 8, 10, 40, 6);
  player->GetComponent<Animator>()->RegisterAnimation("StandingMedium", "spritesheets\\grounded_attacks.png", 8, 10, 46, 9);
  player->GetComponent<Animator>()->RegisterAnimation("StandingHeavy", "spritesheets\\grounded_attacks.png", 8, 10, 55, 13);
  player->GetComponent<Animator>()->RegisterAnimation("JumpingLight", "spritesheets\\jlp.png", 4, 4, 0, 14);
  player->GetComponent<Animator>()->RegisterAnimation("JumpingMedium", "spritesheets\\jlp.png", 4, 4, 0, 14);
  player->GetComponent<Animator>()->RegisterAnimation("JumpingHeavy", "spritesheets\\jlp.png", 4, 4, 0, 14);

  player->GetComponent<Animator>()->Play("Idle", true);

  player->GetComponent<RectColliderD>()->Init(Vector2<double>(xOffset, 0.0),
    Vector2<double>(static_cast<double>(textureSize.x), static_cast<double>(textureSize.y)));
  player->GetComponent<RectColliderD>()->SetStatic(false);

  player->SetScale(Vector2<float>(1.4f, 1.7f));
  player->transform.position.x = xOffset;

  return NetworkPlayer(input, player);
}
#endif

std::shared_ptr<Camera> EntityCreation::CreateCamera()
{
  auto camera = GameManager::Get().CreateEntity<Camera>();
  camera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
  return camera->GetComponent<Camera>();
}
