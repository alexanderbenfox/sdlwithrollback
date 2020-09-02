#include "Components/SFXComponent.h"
#include "Managers/GameManagement.h"
#include "Systems/TimerSystem/TimerContainer.h"

#include "AssetManagement/AnimationCollectionManager.h"

// for hit block sparks sprite sheet data... stupid but ill fix later
SpriteSheet hitblockSparksInfo("sfx\\hitblocksparks.png", 8, 7, true);


void SFXComponent::OnAdd(const EntityID& entity)
{
  _sfxEntity = GameManager::Get().CreateEntity<RenderComponent<GLTexture>, Transform, Animator, TimerContainer>();
  _sfxEntity->SetScale(Vector2<float>(0.75, 0.75f));

  _sfxEntity->GetComponent<Animator>()->animCollectionID = AnimationCollectionManager::Get().GetCollectionID("General");

  static bool SSDataGenerated = false;
  if (!SSDataGenerated)
  {
    hitblockSparksInfo.GenerateSheetInfo();
    SSDataGenerated = true;
  }
}

void SFXComponent::OnRemove(const EntityID& entity)
{
  GameManager::Get().DestroyEntity(_sfxEntity);
}

void SFXComponent::ShowHitSparks(bool directionRight)
{
  if (_subroutine)
    _subroutine->Cancel();

  Vector2<float> size = (Vector2<float>)hitblockSparksInfo.frameSize * _sfxEntity->GetComponent<Transform>()->scale;

  _sfxEntity->GetComponent<Transform>()->position = Vector2<float>(showLocation.x - size.x / 2, showLocation.y - size.y / 2);
  _sfxEntity->GetComponent<Animator>()->Play("HitSparks", false, 2.5f, true);

  // add render properties so that the render system shows it
  _sfxEntity->AddComponent<RenderProperties>();
  _sfxEntity->GetComponent<RenderProperties>()->horizontalFlip = !directionRight;
  int flipModifier = !directionRight ? -1 : 1;
  _sfxEntity->GetComponent<RenderProperties>()->offset = Vector2<int>(flipModifier * 40, 0);

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

  Vector2<float> size = (Vector2<float>)hitblockSparksInfo.frameSize * _sfxEntity->GetComponent<Transform>()->scale;
  
  _sfxEntity->GetComponent<Transform>()->position = Vector2<float>(showLocation.x - size.x / 2, showLocation.y - size.y / 2);
  _sfxEntity->GetComponent<Animator>()->Play("BlockSparks", false, 2.5f, true);

  // add render properties so that the render system shows it
  _sfxEntity->AddComponent<RenderProperties>();
  _sfxEntity->GetComponent<RenderProperties>()->horizontalFlip = !directionRight;
  int flipModifier = !directionRight ? -1 : 1;
  _sfxEntity->GetComponent<RenderProperties>()->offset = Vector2<int>(flipModifier * 110, 0);

  // set action timer
  _subroutine = std::shared_ptr<ActionTimer>(new SimpleActionTimer(
    [this]() { _sfxEntity->RemoveComponent<RenderProperties>(); },
  12));
  _sfxEntity->GetComponent<TimerContainer>()->timings.push_back(_subroutine);
}
