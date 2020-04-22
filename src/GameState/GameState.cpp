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

  auto player = GameManager::Get().CreateEntity<Transform, KeyboardInputHandler, Animator, GraphicRenderer, RenderProperties, Rigidbody, GameActor, RectColliderD, Hurtbox>();

  player->GetComponent<Rigidbody>()->Init(true);

  player->GetComponent<Animator>()->RegisterAnimation("Idle", "spritesheets\\idle_and_walking.png", 6, 6, 0, 10);
  player->GetComponent<Animator>()->RegisterAnimation("WalkF", "spritesheets\\idle_and_walking.png", 6, 6, 10, 12);
  player->GetComponent<Animator>()->RegisterAnimation("WalkB", "spritesheets\\idle_and_walking.png", 6, 6, 22, 11);

  player->GetComponent<Animator>()->RegisterAnimation("Jumping", "spritesheets\\idle_walking_jab_jump_crouch.png", 8, 10, 41, 19);
  player->GetComponent<Animator>()->RegisterAnimation("Falling", "spritesheets\\idle_walking_jab_jump_crouch.png", 8, 10, 60, 13);

  player->GetComponent<Animator>()->RegisterAnimation("Crouching", "spritesheets\\crouching.png", 4, 5, 0, 4);
  player->GetComponent<Animator>()->RegisterAnimation("Crouch", "spritesheets\\crouching.png", 4, 5, 12, 5);

  FrameData testData{0, 0, 0, 15, 6, 1, Vector2<float>(120.0f, -700.0f)};

  player->GetComponent<Animator>()->RegisterAnimation("CrouchingLight", "spritesheets\\grounded_attacks.png", 8, 10, 9, 7);
  FrameData cLP{4, 3, 5, 3, 3, 1, Vector2<float>(120.0f, -100.0f), GameManager::Get().hitstopLight };
  player->GetComponent<Animator>()->GetAnimationByName("CrouchingLight")->AddHitboxEvents("spritesheets\\grounded_attacks_hitboxes.png", cLP, player);

  player->GetComponent<Animator>()->RegisterAnimation("CrouchingMedium", "spritesheets\\grounded_attacks.png", 8, 10, 16, 11);
  FrameData cM{7, 3, 13, 5, 0, 1, Vector2<float>(120.0f, -600.0f), GameManager::Get().hitstopMedium};
  player->GetComponent<Animator>()->GetAnimationByName("CrouchingMedium")->AddHitboxEvents("spritesheets\\grounded_attacks_hitboxes.png", cM, player);

  player->GetComponent<Animator>()->RegisterAnimation("CrouchingHeavy", "spritesheets\\grounded_attacks.png", 8, 10, 27, 11);
  FrameData cH{6, 4, 24, 5, 2, 1, Vector2<float>(120.0f, -900.0f), GameManager::Get().hitstopHeavy};
  player->GetComponent<Animator>()->GetAnimationByName("CrouchingHeavy")->AddHitboxEvents("spritesheets\\grounded_attacks_hitboxes.png", cH, player);

  player->GetComponent<Animator>()->RegisterAnimation("StandingLight", "spritesheets\\grounded_attacks.png", 8, 10, 38, 7);
  FrameData L{4, 2, 7, 2, -2, 1, Vector2<float>(120.0f, -100.0f), GameManager::Get().hitstopLight};
  player->GetComponent<Animator>()->GetAnimationByName("StandingLight")->AddHitboxEvents("spritesheets\\grounded_attacks_hitboxes.png", L, player);

  player->GetComponent<Animator>()->RegisterAnimation("StandingMedium", "spritesheets\\grounded_attacks.png", 8, 10, 45, 9);
  FrameData M{5, 3, 10, 4, 2, 1, Vector2<float>(120.0f, -100.0f), GameManager::Get().hitstopMedium};
  player->GetComponent<Animator>()->GetAnimationByName("StandingMedium")->AddHitboxEvents("spritesheets\\grounded_attacks_hitboxes.png", M, player);

  player->GetComponent<Animator>()->RegisterAnimation("StandingHeavy", "spritesheets\\grounded_attacks.png", 8, 10, 53, 12);
  FrameData H{ 8, 3, 20, 5, 3, 1, Vector2<float>(120.0f, -400.0f), GameManager::Get().hitstopHeavy};
  player->GetComponent<Animator>()->GetAnimationByName("StandingHeavy")->AddHitboxEvents("spritesheets\\grounded_attacks_hitboxes.png", H, player);


  player->GetComponent<Animator>()->RegisterAnimation("JumpingLight", "spritesheets\\jlp.png", 4, 4, 0, 14);
  player->GetComponent<Animator>()->RegisterAnimation("JumpingMedium", "spritesheets\\jlp.png", 4, 4, 0, 14);
  player->GetComponent<Animator>()->RegisterAnimation("JumpingHeavy", "spritesheets\\jlp.png", 4, 4, 0, 14);

  player->GetComponent<Animator>()->RegisterAnimation("Block", "spritesheets\\block_mid_hitstun.png", 8, 7, 0, 4);
  player->GetComponent<Animator>()->RegisterAnimation("LightHitstun", "spritesheets\\block_mid_hitstun.png", 8, 7, 4, 3);
  player->GetComponent<Animator>()->RegisterAnimation("LightHitstun2", "spritesheets\\block_mid_hitstun.png", 8, 7, 37, 4);
  player->GetComponent<Animator>()->RegisterAnimation("MediumHitstun", "spritesheets\\block_mid_hitstun.png", 8, 7, 8, 10);
  player->GetComponent<Animator>()->RegisterAnimation("MediumHitstun2", "spritesheets\\block_mid_hitstun.png", 8, 7, 19, 9);
  player->GetComponent<Animator>()->RegisterAnimation("HeavyHitstun", "spritesheets\\block_mid_hitstun.png", 8, 7, 42, 12);
  player->GetComponent<Animator>()->RegisterAnimation("LaunchHitstun", "spritesheets\\block_mid_hitstun.png", 8, 7, 27, 9);

  player->GetComponent<RectColliderD>()->Init(Vector2<double>(xOffset, 0.0),
    Vector2<double>(xOffset + static_cast<double>(textureSize.x)*.75, static_cast<double>(textureSize.y)));
  player->GetComponent<RectColliderD>()->SetStatic(false);

  player->GetComponent<Hurtbox>()->Init(Vector2<double>(xOffset, 0.0),
    Vector2<double>(xOffset + static_cast<double>(textureSize.x) * .75, static_cast<double>(textureSize.y)));
  player->GetComponent<Hurtbox>()->SetStatic(false);

  player->SetScale(Vector2<float>(1.4f, 1.7f));
  player->GetComponent<Transform>()->position.x = xOffset;

  player->GetComponent<RectColliderD>()->MoveToTransform(*player->GetComponent<Transform>());
  player->GetComponent<Hurtbox>()->MoveToTransform(*player->GetComponent<Transform>());

  return player;
}

#ifdef _WIN32
std::shared_ptr<Entity> EntityCreation::CreateNetworkPlayer(float xOffset)
{
  auto player = GameManager::Get().CreateEntity<>();
  return player;
}
#endif

std::shared_ptr<Camera> EntityCreation::CreateCamera()
{
  auto camera = GameManager::Get().CreateEntity<Camera, Transform>();
  camera->GetComponent<Camera>()->Init(m_nativeWidth, m_nativeHeight);
  return camera->GetComponent<Camera>();
}
