#include "GameState/GameState.h"

#include "GameManagement.h"
#include "Components/Animator.h"
#include "Components/GameActor.h"
#include "Components/Rigidbody.h"
#include "Components/Camera.h"
#include "Components/StateComponent.h"

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

AnimationCollection EntityCreation::RyuAnimations()
{
  AnimationCollection ryuAnimations;
  ryuAnimations.RegisterAnimation("Idle", "spritesheets\\idle_and_walking.png", 6, 6, 0, 10, AnchorPoint::TL);
  ryuAnimations.RegisterAnimation("WalkF", "spritesheets\\idle_and_walking.png", 6, 6, 10, 12, AnchorPoint::TL);
  ryuAnimations.RegisterAnimation("WalkB", "spritesheets\\idle_and_walking.png", 6, 6, 22, 11, AnchorPoint::TR);

  ryuAnimations.RegisterAnimation("Jumping", "spritesheets\\idle_walking_jab_jump_crouch.png", 8, 10, 41, 19, AnchorPoint::BL);
  ryuAnimations.RegisterAnimation("Falling", "spritesheets\\idle_walking_jab_jump_crouch.png", 8, 10, 60, 13, AnchorPoint::BL);

  ryuAnimations.RegisterAnimation("Crouching", "spritesheets\\crouching.png", 4, 5, 0, 4, AnchorPoint::BL);
  ryuAnimations.RegisterAnimation("Crouch", "spritesheets\\crouching.png", 4, 5, 12, 5, AnchorPoint::BL);

  ryuAnimations.RegisterAnimation("CrouchingLight", "spritesheets\\grounded_attacks.png", 8, 10, 9, 7, AnchorPoint::BL);
  FrameData cLP{4, 3, 7, 3, 3, 1, Vector2<float>(120.0f, -100.0f), GameManager::Get().hitstopLight };
  ryuAnimations.AddHitboxEvents("CrouchingLight", "spritesheets\\grounded_attacks_hitboxes.png", cLP);

  ryuAnimations.RegisterAnimation("CrouchingMedium", "spritesheets\\grounded_attacks.png", 8, 10, 16, 11, AnchorPoint::BL);
  FrameData cM{10, 3, 13, 5, 0, 1, Vector2<float>(120.0f, -600.0f), GameManager::Get().hitstopMedium};
  ryuAnimations.AddHitboxEvents("CrouchingMedium", "spritesheets\\grounded_attacks_hitboxes.png", cM);

  ryuAnimations.RegisterAnimation("CrouchingHeavy", "spritesheets\\grounded_attacks.png", 8, 10, 27, 11, AnchorPoint::BL);
  FrameData cH{6, 4, 24, 5, 2, 1, Vector2<float>(120.0f, -900.0f), GameManager::Get().hitstopHeavy};
  ryuAnimations.AddHitboxEvents("CrouchingHeavy", "spritesheets\\grounded_attacks_hitboxes.png", cH);

  ryuAnimations.RegisterAnimation("StandingLight", "spritesheets\\grounded_attacks.png", 8, 10, 38, 7, AnchorPoint::BL);
  FrameData L{4, 2, 7, 3, -2, 1, Vector2<float>(120.0f, -100.0f), GameManager::Get().hitstopLight};
  ryuAnimations.AddHitboxEvents("StandingLight", "spritesheets\\grounded_attacks_hitboxes.png", L);

  ryuAnimations.RegisterAnimation("StandingMedium", "spritesheets\\grounded_attacks.png", 8, 10, 45, 9, AnchorPoint::BL);
  FrameData M{7, 3, 12, 4, 2, 1, Vector2<float>(120.0f, -100.0f), GameManager::Get().hitstopMedium};
  ryuAnimations.AddHitboxEvents("StandingMedium", "spritesheets\\grounded_attacks_hitboxes.png", M);

  ryuAnimations.RegisterAnimation("StandingHeavy", "spritesheets\\grounded_attacks.png", 8, 10, 53, 12, AnchorPoint::BL);
  FrameData H{ 8, 3, 20, 7, -6, 1, Vector2<float>(120.0f, -400.0f), GameManager::Get().hitstopHeavy};
  ryuAnimations.AddHitboxEvents("StandingHeavy", "spritesheets\\grounded_attacks_hitboxes.png", H);


  // special moves - quarter circle forward punch aka hadoken
  ryuAnimations.RegisterAnimation("SpecialMove1", "spritesheets\\grounded_attacks.png", 8, 10, 65, 14, AnchorPoint::BL);
  FrameData Hadouken{ 11, 3, 28, -3, -6, 1, Vector2<float>(400.0f, 100.0f), GameManager::Get().hitstopHeavy };
  ryuAnimations.AddHitboxEvents("SpecialMove1", "spritesheets\\grounded_attacks_hitboxes.png", Hadouken);

  ryuAnimations.RegisterAnimation("JumpingLight", "spritesheets\\jlp.png", 4, 4, 0, 14, AnchorPoint::TL);
  ryuAnimations.RegisterAnimation("JumpingMedium", "spritesheets\\jlp.png", 4, 4, 0, 14, AnchorPoint::TL);
  ryuAnimations.RegisterAnimation("JumpingHeavy", "spritesheets\\jlp.png", 4, 4, 0, 14, AnchorPoint::TL);

  ryuAnimations.RegisterAnimation("Block", "spritesheets\\block_mid_hitstun.png", 8, 7, 0, 4, AnchorPoint::BL);
  ryuAnimations.RegisterAnimation("LightHitstun", "spritesheets\\block_mid_hitstun.png", 8, 7, 4, 3, AnchorPoint::BL);
  ryuAnimations.RegisterAnimation("LightHitstun2", "spritesheets\\block_mid_hitstun.png", 8, 7, 37, 4, AnchorPoint::BL);
  ryuAnimations.RegisterAnimation("MediumHitstun", "spritesheets\\block_mid_hitstun.png", 8, 7, 8, 10, AnchorPoint::BL);
  ryuAnimations.RegisterAnimation("MediumHitstun2", "spritesheets\\block_mid_hitstun.png", 8, 7, 19, 9, AnchorPoint::BL);
  ryuAnimations.RegisterAnimation("HeavyHitstun", "spritesheets\\block_mid_hitstun.png", 8, 7, 42, 12, AnchorPoint::BL);
  ryuAnimations.RegisterAnimation("LaunchHitstun", "spritesheets\\block_mid_hitstun.png", 8, 7, 27, 9, AnchorPoint::BL);

  return ryuAnimations;
}

std::shared_ptr<Entity> EntityCreation::CreateLocalPlayer(float xOffset)
{
  Vector2<int> textureSize = ResourceManager::Get().GetTextureWidthAndHeight("spritesheets\\ryu.png");
  Vector2<double> entitySize(static_cast<double>(textureSize.x)*.75, static_cast<double>(textureSize.y) * .95);

  auto player = GameManager::Get().CreateEntity<Transform, KeyboardInputHandler, Animator, GraphicRenderer, RenderProperties, Rigidbody, GameActor, RectColliderD, Hurtbox, StateComponent>();

  player->GetComponent<Rigidbody>()->Init(true);
  player->GetComponent<Animator>()->SetAnimations(RyuAnimations());

  player->GetComponent<Transform>()->SetWidthAndHeight(entitySize.x, entitySize.y);
  player->GetComponent<RenderProperties>()->baseRenderOffset = (entitySize * (-1.0/2.0));
  player->GetComponent<RenderProperties>()->baseRenderOffset.y -= (static_cast<double>(textureSize.y) * .05);

  player->GetComponent<RectColliderD>()->Init(Vector2<double>::Zero, entitySize);
  player->GetComponent<RectColliderD>()->SetStatic(false);

  player->GetComponent<Hurtbox>()->Init(Vector2<double>::Zero, entitySize);
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
