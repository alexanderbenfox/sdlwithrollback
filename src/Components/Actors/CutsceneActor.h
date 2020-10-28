#pragma once
#include "Components/Animator.h"
#include "Components/RenderComponent.h"

#include "Managers/AnimationCollectionManager.h"

struct CutsceneAction;

class CutsceneActor : public IComponent
{
public:
  CutsceneActor() : IComponent() {}

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
  PlayAnimation(std::string animationName, float speed) : anim(animationName), speed(speed)
  {
    CutsceneAction::isWaiting = false;
  }
  void Begin(Animator* animator, RenderComponent<RenderType>* renderer, RenderProperties* properties) override
  {
    animator->Play(anim, false, speed);
    animator->ChangeListener(this);

    Animation* actionAnimation = GAnimArchive.GetAnimationData(animator->animCollectionID, animator->currentAnimationName);
    // render from the sheet of the new animation
    renderer->SetRenderResource(actionAnimation->GetSheetTexture<RenderType>());
    renderer->sourceRect = actionAnimation->GetFrameSrcRect(0);

    // figure out a better way to do offsets lol
    properties->offset = -GAnimArchive.GetCollection(animator->animCollectionID).GetRenderOffset(anim, properties->horizontalFlip, (int)std::floor(properties->unscaledRenderWidth));
    if (properties->horizontalFlip)
      properties->offset.x *= properties->renderScaling.x;
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
  float speed = 1.0f;
  bool animFinished = false;
};

#include "Systems/TimerSystem/ActionTimer.h"
#include "Systems/TimerSystem/TimerContainer.h"

struct AlphaFader : public CutsceneAction
{
  //! fade time is in seconds
  AlphaFader(float fadeTime, unsigned char startAlpha, unsigned char endAlpha) : time(fadeTime), start(startAlpha), end(endAlpha)
  {
    CutsceneAction::isWaiting = false;
  }
  void Begin(Animator* animator, RenderComponent<RenderType>* renderer, RenderProperties* properties) override
  {
    // target has to be set after
    assert(target != nullptr);
    //! set to start alpha at beginning
    properties->SetDisplayColor(255, 255, 255, start);

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
    time * 1.0f/secPerFrame);
    target->GetComponent<TimerContainer>()->timings.push_back(timer);
  }
  void OnComplete() override {}
  virtual bool CheckEndConditions() override { return complete; }
  virtual bool CheckEndConditions(CutsceneActor* other) override { return false; }

  bool complete = false;

  unsigned char start, end;
  float time;
  std::shared_ptr<ComplexActionTimer> timer = nullptr;
  std::shared_ptr<Entity> target;
};

// just wait for a certain amount of time
struct WaitForTime : public CutsceneAction
{
  //! time is in seconds
  WaitForTime(float time) : time(time)
  {
    CutsceneAction::isWaiting = false;
  }
  void Begin(Animator* animator, RenderComponent<RenderType>* renderer, RenderProperties* properties) override
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
  void OnComplete() override {}
  virtual bool CheckEndConditions() override { return complete; }
  virtual bool CheckEndConditions(CutsceneActor* other) override { return false; }

  bool complete = false;

  float time;
  std::shared_ptr<SimpleActionTimer> timer = nullptr;
  std::shared_ptr<Entity> target;
};
