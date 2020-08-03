#include "Components/SFXComponent.h"
#include "GameManagement.h"
#include "Components/GameActor.h"

static AnimationCollection HitSFXAnimationCollection;
SpriteSheet hitblockSparks("sfx\\hitblocksparks.png", 8, 7, true);

SFXComponent::SFXComponent(std::shared_ptr<Entity> entity) : IComponent(entity)
{
  _sfxEntity = GameManager::Get().CreateEntity<RenderComponent<GLTexture>, Transform, Animator, TimerContainer>();

  _sfxEntity->SetScale(Vector2<float>(0.75, 0.75f));

  hitblockSparks.GenerateSheetInfo();
  HitSFXAnimationCollection.RegisterAnimation("HitSparks", hitblockSparks, 0, 28, AnchorPoint::BL);
  HitSFXAnimationCollection.RegisterAnimation("BlockSparks", hitblockSparks, 28, 28, AnchorPoint::BL);
  _sfxEntity->GetComponent<Animator>()->SetAnimations(&HitSFXAnimationCollection);
}

SFXComponent::~SFXComponent()
{
  GameManager::Get().DestroyEntity(_sfxEntity);
}

void SFXComponent::ShowHitSparks()
{
  if (_subroutine)
    _subroutine->Cancel();

  Vector2<float> size = (Vector2<float>)hitblockSparks.frameSize * _sfxEntity->GetComponent<Transform>()->scale;
  _sfxEntity->GetComponent<Transform>()->position = Vector2<float>(showLocation.x - size.x / 2, showLocation.y - size.y / 2);
  _sfxEntity->GetComponent<Animator>()->Play("HitSparks", false, 2.5f, true);

  // add render properties so that the render system shows it
  _sfxEntity->AddComponent<RenderProperties>();
  _sfxEntity->GetComponent<RenderProperties>()->horizontalFlip = !_owner->GetComponent<StateComponent>()->onLeftSide;
  int flipModifier = !_owner->GetComponent<StateComponent>()->onLeftSide ? -1 : 1;
  _sfxEntity->GetComponent<RenderProperties>()->offset = Vector2<int>(flipModifier * 40, 0);

  // set action timer
  _subroutine = std::shared_ptr<ActionTimer>(new SimpleActionTimer(
    [this]() { _sfxEntity->RemoveComponent<RenderProperties>(); },
  12));
  _sfxEntity->GetComponent<TimerContainer>()->timings.push_back(_subroutine);
}

void SFXComponent::ShowBlockSparks()
{
  if (_subroutine)
    _subroutine->Cancel();

  Vector2<float> size = (Vector2<float>)hitblockSparks.frameSize * _sfxEntity->GetComponent<Transform>()->scale;
  _sfxEntity->GetComponent<Transform>()->position = Vector2<float>(showLocation.x - size.x / 2, showLocation.y - size.y / 2);
  _sfxEntity->GetComponent<Animator>()->Play("BlockSparks", false, 2.5f, true);

  // add render properties so that the render system shows it
  _sfxEntity->AddComponent<RenderProperties>();
  _sfxEntity->GetComponent<RenderProperties>()->horizontalFlip = !_owner->GetComponent<StateComponent>()->onLeftSide;
  int flipModifier = !_owner->GetComponent<StateComponent>()->onLeftSide ? -1 : 1;
  _sfxEntity->GetComponent<RenderProperties>()->offset = Vector2<int>(flipModifier * 110, 0);

  // set action timer
  _subroutine = std::shared_ptr<ActionTimer>(new SimpleActionTimer(
    [this]() { _sfxEntity->RemoveComponent<RenderProperties>(); },
  12));
  _sfxEntity->GetComponent<TimerContainer>()->timings.push_back(_subroutine);
}
