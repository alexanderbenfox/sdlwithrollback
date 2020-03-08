#include "GameState/GameState.h"

#include "GameManagement.h"
#include "Components/Animator.h"
#include "Components/GameActor.h"
#include "Components/Rigidbody.h"
#include "Components/Camera.h"

void LocalMatch::ProcessRawInputs(SDL_Event* localInput)
{
  /*InputState rawInput = _player1.input->HandleInput(localInput);
  GameContext contexts = GetActiveContext(&_player1);
  _player1.actor->GetComponent<GameActor>()->EvaluateInputContext(rawInput, contexts);
  
  rawInput = _player2.input->HandleInput(localInput);
  contexts = GetActiveContext(&_player2);
  _player2.actor->GetComponent<GameActor>()->EvaluateInputContext(rawInput, contexts);*/
}

/*GameContext LocalMatch::GetActiveContext(Player* player)
{
  GameContext context;

  if (auto rb = player->actor->GetComponent<Rigidbody>())
  {
    context.collision = rb->_lastCollisionSide;
  }
  else
  {
    context.collision = CollisionSide::NONE;
  }

  LocalPlayer& otherPlayer = player == &_player1 ? _player2 : _player1;
  context.onLeftSide = player->GetCenterX() < otherPlayer.GetCenterX();

  return context;
}*/

std::shared_ptr<Entity> EntityCreation::CreateLocalPlayer(float xOffset)
{
  Vector2<int> textureSize = ResourceManager::Get().GetTextureWidthAndHeight("spritesheets\\ryu.png");

  auto player = GameManager::Get().CreateEntity<Transform, KeyboardInputHandler, AnimationRenderer, Rigidbody, GameActor, RectColliderD>();

  player->GetComponent<Rigidbody>()->Init(true);

  player->GetComponent<AnimationRenderer>()->RegisterAnimation("Idle", "spritesheets\\idle_and_walking.png", 6, 6, 0, 10);
  player->GetComponent<AnimationRenderer>()->RegisterAnimation("WalkF", "spritesheets\\idle_and_walking.png", 6, 6, 10, 12);
  player->GetComponent<AnimationRenderer>()->RegisterAnimation("WalkB", "spritesheets\\idle_and_walking.png", 6, 6, 22, 11);

  player->GetComponent<AnimationRenderer>()->RegisterAnimation("Jumping", "spritesheets\\idle_walking_jab_jump_crouch.png", 8, 10, 41, 19);
  player->GetComponent<AnimationRenderer>()->RegisterAnimation("Falling", "spritesheets\\idle_walking_jab_jump_crouch.png", 8, 10, 60, 13);

  player->GetComponent<AnimationRenderer>()->RegisterAnimation("Crouching", "spritesheets\\crouching.png", 4, 5, 0, 4);
  player->GetComponent<AnimationRenderer>()->RegisterAnimation("Crouch", "spritesheets\\crouching.png", 4, 5, 12, 5);

  player->GetComponent<AnimationRenderer>()->RegisterAnimation("CrouchingLight", "spritesheets\\grounded_attacks.png", 8, 10, 9, 7);
  player->GetComponent<AnimationRenderer>()->RegisterAnimation("CrouchingMedium", "spritesheets\\grounded_attacks.png", 8, 10, 16, 11);
  player->GetComponent<AnimationRenderer>()->RegisterAnimation("CrouchingHeavy", "spritesheets\\grounded_attacks.png", 8, 10, 27, 11);
  player->GetComponent<AnimationRenderer>()->RegisterAnimation("StandingLight", "spritesheets\\grounded_attacks.png", 8, 10, 40, 6);
  player->GetComponent<AnimationRenderer>()->RegisterAnimation("StandingMedium", "spritesheets\\grounded_attacks.png", 8, 10, 46, 9);
  player->GetComponent<AnimationRenderer>()->RegisterAnimation("StandingHeavy", "spritesheets\\grounded_attacks.png", 8, 10, 55, 13);
  player->GetComponent<AnimationRenderer>()->RegisterAnimation("JumpingLight", "spritesheets\\jlp.png", 4, 4, 0, 14);
  player->GetComponent<AnimationRenderer>()->RegisterAnimation("JumpingMedium", "spritesheets\\jlp.png", 4, 4, 0, 14);
  player->GetComponent<AnimationRenderer>()->RegisterAnimation("JumpingHeavy", "spritesheets\\jlp.png", 4, 4, 0, 14);

  player->GetComponent<AnimationRenderer>()->Play("Idle", true, xOffset != 0);

  player->GetComponent<RectColliderD>()->Init(Vector2<double>(xOffset, 0.0),
    Vector2<double>(xOffset + static_cast<double>(textureSize.x)*.75, static_cast<double>(textureSize.y)));
  player->GetComponent<RectColliderD>()->SetStatic(false);

  player->SetScale(Vector2<float>(1.4f, 1.7f));
  player->GetComponent<Transform>()->position.x = xOffset;
  player->GetComponent<RectColliderD>()->MoveToTransform(*player->GetComponent<Transform>());

  return player;
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

  player->GetComponent<Animator>()->Play("Idle", true, xOffset != 0);

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
  auto camera = GameManager::Get().CreateEntity<Camera, Transform>();
  camera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
  return camera->GetComponent<Camera>();
}
