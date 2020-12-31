#include "CutsceneActor.h"
#include "Components/RenderComponent.h"
#include "Systems/ActionSystems/EnactActionSystem.h"

void CutsceneActor::SetActionList(CutsceneAction** actionArray, int size)
{
  _actionQueue = actionArray;
  _numActions = size;
}

CutsceneAction* CutsceneActor::ActionListPop()
{
  started = true;
  if (actionStage < _numActions)
  {
    currentAction = _actionQueue[actionStage];
    actionStage++;
    return currentAction;
  }
  else if (actionStage == _numActions)
  {
    // increment action stage here to indicate last stage has completed
    actionStage++;
  }
  currentAction = nullptr;
  return currentAction;
}

void PlayAnimation::Begin(EntityID actor)
{
  const RenderProperties& props = ComponentArray<RenderProperties>::Get().GetComponent(actor);
  EnactAnimationActionSystem::PlayAnimation(actor, anim, false, speed, true, !props.horizontalFlip);
  Animator& animator = ComponentArray<Animator>::Get().GetComponent(actor);
  animator.ChangeListener(this);
}

void AlphaFader::Begin(EntityID actor)
{
  RenderProperties& properties = ComponentArray<RenderProperties>::Get().GetComponent(actor);
  // target has to be set after
  assert(target != nullptr);
  //! set to start alpha at beginning
  properties.SetDisplayColor(255, 255, 255, start);

  timer = std::make_shared<ComplexActionTimer>(
    [this](float curr, float total)
    {
      //! lerp display alpha
      target->GetComponent<RenderProperties>()->SetDisplayColor(255, 255, 255, start + static_cast<unsigned char>(static_cast<float>(end - start) * curr / total));
    },
    [this]() {
      target->GetComponent<RenderProperties>()->SetDisplayColor(255, 255, 255, end);
      complete = true;
    },
      time * 1.0f / secPerFrame);
  target->GetComponent<TimerContainer>()->timings.push_back(timer);
}

void WaitForTime::Begin(EntityID actor)
{
  // target has to be set after
  assert(target != nullptr);

  timer = std::make_shared<SimpleActionTimer>(
    [this]() {
      complete = true;
    },
    time * 1.0f / secPerFrame);
  target->GetComponent<TimerContainer>()->timings.push_back(timer);
}
