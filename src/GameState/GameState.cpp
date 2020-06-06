#include "GameState/GameState.h"

#include "GameManagement.h"
#include "Components/Animator.h"
#include "Components/GameActor.h"
#include "Components/Rigidbody.h"
#include "Components/Camera.h"
#include "Components/StateComponent.h"
#include "Components/RenderComponent.h"

#include "ResourceManager.h"

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
  SpriteSheet idleAndWalking("spritesheets\\idle_and_walking.png", 6, 6);
  SpriteSheet jumpFall("spritesheets\\idle_walking_jab_jump_crouch.png", 8, 10);
  SpriteSheet crouching("spritesheets\\crouching.png", 4, 5);
  SpriteSheet groundedAttacks("spritesheets\\grounded_attacks.png", 8, 10);

  AnimationCollection ryuAnimations;
  ryuAnimations.RegisterAnimation("Idle", idleAndWalking, 0, 10, AnchorPoint::TL);
  ryuAnimations.RegisterAnimation("WalkF", idleAndWalking, 10, 12, AnchorPoint::TL);
  ryuAnimations.RegisterAnimation("WalkB", idleAndWalking, 22, 11, AnchorPoint::TR);

  ryuAnimations.RegisterAnimation("Jumping", jumpFall, 41, 19, AnchorPoint::BL);
  ryuAnimations.RegisterAnimation("Falling", jumpFall, 60, 13, AnchorPoint::BL);

  ryuAnimations.RegisterAnimation("Crouching", crouching, 0, 4, AnchorPoint::BL);
  ryuAnimations.RegisterAnimation("Crouch", crouching, 12, 5, AnchorPoint::BL);

  ryuAnimations.RegisterAnimation("CrouchingLight", groundedAttacks, 9, 7, AnchorPoint::BL);
  FrameData cLP{4, 3, 7, 3, 3, 1, Vector2<float>(120.0f, -100.0f), GameManager::Get().hitstopLight };
  ryuAnimations.AddHitboxEvents("CrouchingLight", "spritesheets\\grounded_attacks_hitboxes.png", cLP);

  ryuAnimations.RegisterAnimation("CrouchingMedium", groundedAttacks, 16, 11, AnchorPoint::BL);
  FrameData cM{10, 3, 13, 5, 0, 1, Vector2<float>(120.0f, -600.0f), GameManager::Get().hitstopMedium};
  ryuAnimations.AddHitboxEvents("CrouchingMedium", "spritesheets\\grounded_attacks_hitboxes.png", cM);

  ryuAnimations.RegisterAnimation("CrouchingHeavy", groundedAttacks, 27, 11, AnchorPoint::BL);
  FrameData cH{6, 4, 24, 5, 2, 1, Vector2<float>(120.0f, -900.0f), GameManager::Get().hitstopHeavy};
  ryuAnimations.AddHitboxEvents("CrouchingHeavy", "spritesheets\\grounded_attacks_hitboxes.png", cH);

  ryuAnimations.RegisterAnimation("StandingLight", groundedAttacks, 38, 7, AnchorPoint::BL);
  FrameData L{4, 2, 7, 3, -2, 1, Vector2<float>(120.0f, -100.0f), GameManager::Get().hitstopLight};
  ryuAnimations.AddHitboxEvents("StandingLight", "spritesheets\\grounded_attacks_hitboxes.png", L);

  ryuAnimations.RegisterAnimation("StandingMedium", groundedAttacks, 45, 9, AnchorPoint::BL);
  FrameData M{7, 3, 12, 4, 2, 1, Vector2<float>(120.0f, -100.0f), GameManager::Get().hitstopMedium};
  ryuAnimations.AddHitboxEvents("StandingMedium", "spritesheets\\grounded_attacks_hitboxes.png", M);

  ryuAnimations.RegisterAnimation("StandingHeavy", groundedAttacks, 53, 12, AnchorPoint::BL);
  FrameData H{ 8, 3, 20, 7, -6, 1, Vector2<float>(120.0f, -400.0f), GameManager::Get().hitstopHeavy};
  ryuAnimations.AddHitboxEvents("StandingHeavy", "spritesheets\\grounded_attacks_hitboxes.png", H);


  // special moves - quarter circle forward punch aka hadoken
  ryuAnimations.RegisterAnimation("SpecialMove1", groundedAttacks, 65, 14, AnchorPoint::BL);
  FrameData Hadouken{ 11, 3, 28, -3, -6, 1, Vector2<float>(400.0f, 100.0f), GameManager::Get().hitstopHeavy };
  ryuAnimations.AddHitboxEvents("SpecialMove1", "spritesheets\\grounded_attacks_hitboxes.png", Hadouken);

  SpriteSheet jumpingAttacks("spritesheets\\jlp.png", 4, 4);

  ryuAnimations.RegisterAnimation("JumpingLight", jumpingAttacks, 0, 14, AnchorPoint::TL);
  ryuAnimations.RegisterAnimation("JumpingMedium", jumpingAttacks, 0, 14, AnchorPoint::TL);
  ryuAnimations.RegisterAnimation("JumpingHeavy", jumpingAttacks, 0, 14, AnchorPoint::TL);

  SpriteSheet blockAndHitstun("spritesheets\\block_mid_hitstun.png", 8, 7);

  ryuAnimations.RegisterAnimation("Block", blockAndHitstun, 0, 4, AnchorPoint::BL);
  ryuAnimations.RegisterAnimation("LightHitstun", blockAndHitstun, 4, 3, AnchorPoint::BL);
  ryuAnimations.RegisterAnimation("LightHitstun2", blockAndHitstun, 37, 4, AnchorPoint::BL);
  ryuAnimations.RegisterAnimation("MediumHitstun", blockAndHitstun, 8, 10, AnchorPoint::BL);
  ryuAnimations.RegisterAnimation("MediumHitstun2", blockAndHitstun, 19, 9, AnchorPoint::BL);
  ryuAnimations.RegisterAnimation("HeavyHitstun", blockAndHitstun, 42, 12, AnchorPoint::BL);
  ryuAnimations.RegisterAnimation("LaunchHitstun", blockAndHitstun, 27, 9, AnchorPoint::BL);

  return ryuAnimations;
}

std::shared_ptr<Entity> EntityCreation::CreateLocalPlayer(float xOffset)
{
  Vector2<int> textureSize = ResourceManager::Get().GetTextureWidthAndHeight("spritesheets\\ryu.png");
  Vector2<double> entitySize(static_cast<double>(textureSize.x)*.75, static_cast<double>(textureSize.y) * .95);

  auto player = GameManager::Get().CreateEntity<Transform, KeyboardInputHandler, Animator, RenderComponent<RenderType>, RenderProperties, Rigidbody, GameActor, DynamicCollider, Hurtbox, StateComponent>();

  player->GetComponent<Rigidbody>()->Init(true);
  player->GetComponent<Animator>()->SetAnimations(RyuAnimations());

  player->GetComponent<Transform>()->SetWidthAndHeight(entitySize.x, entitySize.y);
  player->GetComponent<RenderProperties>()->baseRenderOffset = (entitySize * (-1.0/2.0));
  player->GetComponent<RenderProperties>()->baseRenderOffset.y -= (static_cast<double>(textureSize.y) * .05);

  player->GetComponent<DynamicCollider>()->Init(Vector2<double>::Zero, entitySize);
  player->GetComponent<Hurtbox>()->Init(Vector2<double>::Zero, entitySize);

  player->SetScale(Vector2<float>(1.4f, 1.7f));
  player->GetComponent<Transform>()->position.x = xOffset;

  player->GetComponent<DynamicCollider>()->MoveToTransform(*player->GetComponent<Transform>());
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
