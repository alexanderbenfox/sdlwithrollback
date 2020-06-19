#pragma once
#include "Components/Animator.h"
#include "Components/RenderComponent.h"

struct CutsceneAction;

class CutsceneActor : public IComponent
{
public:
  //!
  CutsceneActor(std::shared_ptr<Entity> owner) : IComponent(owner) {}
  //!
  ~CutsceneActor() {}

  void SetActionList(CutsceneAction** actionArray, int size)
  {
    _actionQueue = actionArray;
    _numActions = size;
  }

  CutsceneAction* ActionListPop()
  {
    started = true;
    if(actionStage < _numActions)
    {
      currentAction = _actionQueue[actionStage];
      actionStage++;
      return currentAction;
    }
    else if(actionStage == _numActions)
    {
      // increment action stage here to indicate last stage has completed
      actionStage++;
    }
    currentAction = nullptr;
    return currentAction;
  }

  bool started = false;

  CutsceneAction* currentAction = nullptr;

  int actionStage = 0;

protected:
  CutsceneAction** _actionQueue = nullptr;
  int _numActions = 0;

};

struct CutsceneAction
{
  virtual void Begin(Animator*, RenderComponent<RenderType>*, RenderProperties*) = 0;
  virtual void OnComplete() = 0;
  virtual bool CheckEndConditions() = 0; 
  virtual bool CheckEndConditions(CutsceneActor*) = 0;

  bool isWaiting;
};

struct Wait : public CutsceneAction
{
  Wait(int waitUntilStage) : condition(waitUntilStage)
  {
    CutsceneAction::isWaiting = true;
  }
  void Begin(Animator* animator, RenderComponent<RenderType>* renderer,  RenderProperties* properties) override {}
  void OnComplete() override {}
  virtual bool CheckEndConditions() override { return false; }
  virtual bool CheckEndConditions(CutsceneActor* other) override
  {
    return other->actionStage == condition;
  }

  int condition;
};

struct PlayAnimation : public CutsceneAction, public IAnimatorListener
{
  PlayAnimation(std::string animationName) : anim(animationName)
  {
    CutsceneAction::isWaiting = false;
  }
  void Begin(Animator* animator, RenderComponent<RenderType>* renderer, RenderProperties* properties) override
  {
    animator->Play(anim, false, false);
    animator->ChangeListener(this);

    auto& actionAnimation = animator->GetCurrentAnimation();
    // render from the sheet of the new animation
    renderer->SetRenderResource(actionAnimation.GetSheetTexture<RenderType>());
    renderer->sourceRect = actionAnimation.GetFrameSrcRect(0);

    // do everything facing right now and ill fix this eventually
    properties->horizontalFlip = false;
    properties->offset = -animator->AnimationLib()->GetRenderOffset(anim, false, 0);
  }
  void OnComplete() override {}
  virtual bool CheckEndConditions() override { return animFinished; }
  virtual bool CheckEndConditions(CutsceneActor* other) override { return false; }

  //! 
  virtual void OnAnimationComplete(const std::string& completedAnimation) override
  {
    animFinished = true;
  }

  std::string anim;
  bool animFinished = false;
};
