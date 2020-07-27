#include "Components/SFXComponent.h"
#include "GameManagement.h"
#include "Components/GameActor.h"

static AnimationCollection HitSFXAnimationCollection;
SpriteSheet hitSparks("sfx\\lighthit.png", 6, 5, true);

SFXComponent::SFXComponent(std::shared_ptr<Entity> entity) : IComponent(entity)
{
  _sfxEntity = GameManager::Get().CreateEntity<RenderComponent<GLTexture>, Transform, Animator, TimerContainer>();

  _sfxEntity->SetScale(Vector2<float>(0.75, 0.75f));

  hitSparks.GenerateSheetInfo();
  HitSFXAnimationCollection.RegisterAnimation("HitSparks", hitSparks, 0, 28, AnchorPoint::BL);
  _sfxEntity->GetComponent<Animator>()->SetAnimations(&HitSFXAnimationCollection);
}

SFXComponent::~SFXComponent()
{
  GameManager::Get().DestroyEntity(_sfxEntity);
}

void SFXComponent::ShowHitSparks(const Vector2<float>& location)
{
  if (_subroutine)
    _subroutine->Cancel();

  Vector2<float> size = (Vector2<float>)hitSparks.frameSize * _sfxEntity->GetComponent<Transform>()->scale;
  _sfxEntity->GetComponent<Transform>()->position = Vector2<float>(location.x - size.x / 2, location.y - size.y / 2);
  _sfxEntity->GetComponent<Animator>()->Play("HitSparks", false, !_owner->GetComponent<StateComponent>()->onLeftSide, 2.5f, true);

  _sfxEntity->AddComponent<RenderProperties>();
  _subroutine = std::shared_ptr<ActionTimer>(new SimpleActionTimer(
    [this]() { _sfxEntity->RemoveComponent<RenderProperties>(); },
  12));

  _sfxEntity->GetComponent<TimerContainer>()->timings.push_back(_subroutine);
}

void SFXComponent::ShowBlockSparks(const Vector2<float>& location)
{

}