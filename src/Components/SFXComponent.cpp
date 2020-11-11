#include "Components/SFXComponent.h"
#include "Managers/GameManagement.h"
#include "Systems/TimerSystem/TimerContainer.h"

#include "Managers/AnimationCollectionManager.h"
#include "Systems/ActionSystems/EnactActionSystem.h"

// for hit block sparks sprite sheet data... stupid but ill fix later
//SpriteSheet hitblockSparksInfo("sfx\\hitblocksparks.png", 8, 7, true);


void SFXComponent::OnAdd(const EntityID& entity)
{
  _sfxEntity = GameManager::Get().CreateEntity<RenderComponent<GLTexture>, Transform, Animator, TimerContainer>();
  _sfxEntity->SetScale(Vector2<float>(0.75, 0.75f));

  _sfxEntity->GetComponent<Animator>()->animCollectionID = AnimationCollectionManager::Get().GetCollectionID("General");
}

void SFXComponent::OnRemove(const EntityID& entity)
{
  GameManager::Get().DestroyEntity(_sfxEntity);
}

void SFXComponent::ShowHitSparks(bool directionRight)
{
  if (_subroutine)
    _subroutine->Cancel();

  //Vector2<float> size = (Vector2<float>)hitblockSparksInfo.frameSize * _sfxEntity->GetComponent<Transform>()->scale;

  _sfxEntity->GetComponent<Transform>()->position = showLocation;//Vector2<float>(showLocation.x - size.x / 2, showLocation.y - size.y / 2);

  // add render properties so that the render system shows it
  _sfxEntity->AddComponent<RenderProperties>();
  EnactAnimationActionSystem::PlayAnimation(_sfxEntity->GetID(), "HitSparks", false, 2.5f, true, directionRight);

  // set action timer
  _subroutine = std::shared_ptr<ActionTimer>(new SimpleActionTimer(
    [this]() { _sfxEntity->RemoveComponent<RenderProperties>(); },
  12));
  _sfxEntity->GetComponent<TimerContainer>()->timings.push_back(_subroutine);
}

void SFXComponent::ShowBlockSparks(bool directionRight)
{
  if (_subroutine)
    _subroutine->Cancel();

  //Vector2<float> size = (Vector2<float>)hitblockSparksInfo.frameSize * _sfxEntity->GetComponent<Transform>()->scale;
  
  _sfxEntity->GetComponent<Transform>()->position = showLocation;//Vector2<float>(showLocation.x - size.x / 2, showLocation.y - size.y / 2);
  _sfxEntity->AddComponent<RenderProperties>();
  EnactAnimationActionSystem::PlayAnimation(_sfxEntity->GetID(), "BlockSparks", false, 2.5f, true, directionRight);

  // add render properties so that the render system shows it

  // set action timer
  _subroutine = std::shared_ptr<ActionTimer>(new SimpleActionTimer(
    [this]() { _sfxEntity->RemoveComponent<RenderProperties>(); },
  12));
  _sfxEntity->GetComponent<TimerContainer>()->timings.push_back(_subroutine);
}
