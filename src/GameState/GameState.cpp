#include "GameState/GameState.h"

#include "GameManagement.h"
#include "Components/Animator.h"
#include "Components/GameActor.h"
#include "Components/Rigidbody.h"
#include "Components/Camera.h"
#include "Components/StateComponent.h"
#include "AssetManagement/AnimationAsset.h"

void LocalMatch::ProcessRawInputs(SDL_Event* localInput)
{
  /*InputState rawInput = _player1.input->HandleInput(localInput);
  GameContext contexts = GetActiveContext(&_player1);
  _player1.actor->GetComponent<GameActor>()->EvaluateInputContext(rawInput, contexts);
  
  rawInput = _player2.input->HandleInput(localInput);
  contexts = GetActiveContext(&_player2);
  _player2.actor->GetComponent<GameActor>()->EvaluateInputContext(rawInput, contexts);*/
}

std::shared_ptr<Entity> EntityCreation::CreateLocalPlayer(float xOffset)
{
  Vector2<int> textureSize = ResourceManager::Get().GetTextureWidthAndHeight("spritesheets\\ryu.png");
  Vector2<double> entitySize(static_cast<double>(textureSize.x)*.75, static_cast<double>(textureSize.y) * .95);

  auto player = GameManager::Get().CreateEntity<Transform, KeyboardInputHandler, Animator, GraphicRenderer, RenderProperties, Rigidbody, GameActor, DynamicCollider, Hurtbox, StateComponent>();

  player->GetComponent<Rigidbody>()->Init(true);
  player->GetComponent<Animator>()->SetAnimations(&AnimationAsset::RyuAnimations());

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
