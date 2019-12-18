#include "Actions.h"
#include "Entity.h"

#include "Components/Sprite.h"
#include "Components/ActionController.h"
#include "Components/Physics.h"

//______________________________________________________________________________
AnimatedAction::AnimatedAction(const std::string& animation, Entity* actor) : _animation(animation)
{
  if (auto animator = actor->GetComponent<Animator>())
  {
    animator->Play(animation, false);
  }
}


//______________________________________________________________________________
void AnimatedAction::OnUpdate(Entity* actor)
{
  // non-moving command so just make sure you arent moving
  if (actor->GetComponent<Physics>())
  {
    actor->GetComponent<Physics>()->_vel.x = 0;
  }

  // is polling or registering and unregistering listener function better??
  if (auto animator = actor->GetComponent<Animator>())
  {
    int currentAnimationFrame = animator->GetAnimationByName(_animation)->GetFrameCount() - 1;

    if (currentAnimationFrame == animator->GetShowingFrame())
    {
      if (auto ac = actor->GetComponent<ActionController>())
        ac->OnActionComplete(this);
    }
  }
}

bool AnimatedAction::HandleInput(InputState bttn, Entity* actor)
{
  // Is there an action taken on input? Say its a dial combo, this is where that input would be handled
  // return true because we don't want any action taken by the GameActor component
  return true;
}