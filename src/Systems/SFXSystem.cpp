#include "Systems/SFXSystem.h"
#include "Managers/GameManagement.h"
#include "Managers/AnimationCollectionManager.h"
#include "Systems/ActionSystems/EnactActionSystem.h"
#include "Systems/TimerSystem/TimerContainer.h"
#include "Systems/DestroyEntitiesSystem.h"
#include "Components/RenderComponent.h"
#include "Components/Animator.h"

//______________________________________________________________________________
void SFXSystem::DoTick()
{
  ComponentArray<SFXComponent>::Get().ForEach([](SFXComponent& sfx)
  {
    // Lazy-init: create the child SFX entity on first tick
    if (sfx.needsInit)
    {
      auto sfxEntity = GameManager::Get().CreateEntity<RenderComponent<RenderType>, Transform, Animator, TimerContainer, DestroyOnSceneEnd>();
      sfxEntity->SetScale(Vector2<float>(0.75f, 0.75f));
      sfxEntity->GetComponent<Animator>()->animCollectionID = AnimationCollectionManager::Get().GetCollectionID("General");
      sfx.sfxEntityID = sfxEntity->GetID();
      sfx.needsInit = false;
    }

    // Process pending spark requests
    if (sfx.pending == SFXComponent::Request::None)
      return;

    auto sfxEntity = GameManager::Get().GetEntityByID(sfx.sfxEntityID);
    if (!sfxEntity)
      return;

    if (sfx.subroutine)
      sfx.subroutine->Cancel();

    sfxEntity->GetComponent<Transform>()->position = sfx.showLocation;
    sfxEntity->AddComponent<RenderProperties>();

    const char* animName = (sfx.pending == SFXComponent::Request::HitSparks) ? "HitSparks" : "BlockSparks";
    EnactAnimationActionSystem::PlayAnimation(sfx.sfxEntityID, animName, false, 2.5f, true, sfx.pendingDirectionRight);

    // Hide the SFX entity after 12 frames
    EntityID id = sfx.sfxEntityID;
    sfx.subroutine = std::make_shared<SimpleActionTimer>(
      [id]() { GameManager::Get().GetEntityByID(id)->RemoveComponent<RenderProperties>(); },
      12);
    sfxEntity->GetComponent<TimerContainer>()->timings.push_back(sfx.subroutine);

    sfx.pending = SFXComponent::Request::None;
  });
}

//______________________________________________________________________________
void SFXSystem::Cleanup(SFXComponent& sfx)
{
  if (sfx.sfxEntityID != 0)
  {
    GameManager::Get().DestroyEntity(sfx.sfxEntityID);
    sfx.sfxEntityID = 0;
  }
}
