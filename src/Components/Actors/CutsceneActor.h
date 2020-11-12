#pragma once
#include "Components/Animator.h"
#include "Systems/TimerSystem/TimerContainer.h"

struct CutsceneAction;

class CutsceneActor : public IComponent
{
public:
  CutsceneActor() : IComponent() {}

  void SetActionList(CutsceneAction** actionArray, int size);

  CutsceneAction* ActionListPop();

  bool started = false;

  CutsceneAction* currentAction = nullptr;

  int actionStage = 0;

protected:
  CutsceneAction** _actionQueue = nullptr;
  int _numActions = 0;

};

struct CutsceneAction
{
  virtual void Begin(EntityID actor) = 0;
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
  void Begin(EntityID actor) override {}
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
  void Begin(EntityID actor) override;
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

struct AlphaFader : public CutsceneAction
{
  //! fade time is in seconds
  AlphaFader(float fadeTime, unsigned char startAlpha, unsigned char endAlpha) : time(fadeTime), start(startAlpha), end(endAlpha)
  {
    CutsceneAction::isWaiting = false;
  }
  void Begin(EntityID actor) override;
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
  void Begin(EntityID actor) override;
  void OnComplete() override {}
  virtual bool CheckEndConditions() override { return complete; }
  virtual bool CheckEndConditions(CutsceneActor* other) override { return false; }

  bool complete = false;

  float time;
  std::shared_ptr<SimpleActionTimer> timer = nullptr;
  std::shared_ptr<Entity> target;
};
