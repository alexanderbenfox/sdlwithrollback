#include "StateMachine/AttackAction.h"

#include "Components/AttackStateComponent.h"

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
AttackAction<Stance, Action>::~AttackAction()
{
  // make sure this state component is removed
  ListenedAction::_listener->GetOwner()->RemoveComponent<AttackStateComponent>();
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
void AttackAction<Stance, Action>::Enact(Entity* actor)
{
  AnimatedAction<Stance, Action>::Enact(actor);
  if (auto animator = actor->GetComponent<Animator>())
  {
    if (animator->AnimationLib()->GetAnimation(AnimatedAction<Stance, Action>::_animation) && animator->AnimationLib()->GetEventList(AnimatedAction<Stance, Action>::_animation))
    {
      actor->AddComponent<AttackStateComponent>();
      actor->GetComponent<AttackStateComponent>()->Init(animator->AnimationLib()->GetAnimation(AnimatedAction<Stance, Action>::_animation), animator->AnimationLib()->GetEventList(AnimatedAction<Stance, Action>::_animation));
    }
  }
}

//______________________________________________________________________________
template <StanceState Stance, ActionState Action>
void AttackAction<Stance, Action>::OnActionComplete()
{
  ListenedAction::_listener->GetOwner()->RemoveComponent<AttackStateComponent>();
  StateLockedAnimatedAction<Stance, Action>::OnActionComplete();
}
