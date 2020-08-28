#pragma once
#include "Core/Interfaces/Serializable.h"
#include <functional>

//______________________________________________________________________________
class ActionTimer : public ISerializable
{
public:
  ActionTimer(int duration) :
    playTime(0),
    currFrame(0),
    _totalFrames(duration) {}

  virtual ~ActionTimer() = default;
  //! num frames the total action goes for
  int const Duration() { return _totalFrames; }

  //! Serializing functions (might not ever use)
  virtual void Serialize(std::ostream& os) const override;
  virtual void Deserialize(std::istream& is) override;
  std::string Log() override { return ""; }

  //! gets whether or not this action has been cancelled preemptively
  virtual bool const Cancelled() = 0;
  //! callback for when the timer completes
  virtual void OnComplete() = 0;
  //! cancels the action timer
  virtual void Cancel() = 0;
  //! callback for on each frame update
  virtual void Update() = 0;

  //! how long has this been playing in real seconds
  float playTime;
  //! what frame of timer is this one
  int currFrame;

protected:
  //! total length of action timer in frames
  int _totalFrames;

};

//______________________________________________________________________________
//! Drives the animation - only calls the action's on complete function no update
class SimpleActionTimer : public ActionTimer
{
public:
  typedef std::function<void()> CompleteFunc;

  SimpleActionTimer(CompleteFunc onComplete, int duration) :
    _callback(onComplete),
    _cancelled(false),
    ActionTimer(duration) {}

  //! Return cancelled status
  virtual bool const Cancelled() override { return _cancelled; }
  //! If it hasn't been cancelled yet, run the callback
  virtual void OnComplete() override;
  //! Set this to cancelled so it doesn't run callback
  virtual void Cancel() override { _cancelled = true; }
  //! nothing happens on update
  virtual void Update() override {}

protected:
  CompleteFunc _callback;
  bool _cancelled;

};

//______________________________________________________________________________
class ComplexActionTimer : public SimpleActionTimer
{
public:
  typedef std::function<void(float, float)> TFunction;
  ComplexActionTimer(TFunction updater, CompleteFunc onComplete, int duration) :
    _updater(updater),
    SimpleActionTimer(onComplete, duration) {}

  //! updates based on the length of animation play
  virtual void Update() override;

protected:
  TFunction _updater;

};
