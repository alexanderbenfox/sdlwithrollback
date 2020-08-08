#pragma once
#include <functional>

//______________________________________________________________________________
class ActionTimer
{
public:
  ActionTimer(int duration) :
    playTime(0),
    currFrame(0),
    _totalFrames(duration) {}

  virtual ~ActionTimer() = default;
  //! num frames the total action goes for
  int const Duration() { return _totalFrames; }

  friend std::ostream& operator<<(std::ostream& os, const ActionTimer& tm)
  {
    os << tm.playTime;
    os << tm.currFrame;
    os << tm._totalFrames;
    return os;
  }

  friend std::istream& operator>>(std::istream& is, ActionTimer& tm)
  {
    is >> tm.playTime;
    is >> tm.currFrame;
    is >> tm._totalFrames;
    return is;
  }



  //! gets whether or not this action has been cancelled preemptively
  virtual bool const Cancelled() = 0;
  //! callback for when the timer completes
  virtual void OnComplete() = 0;
  //! cancels the action timer
  virtual void Cancel() = 0;
  //!
  virtual void Update() = 0;

  //!
  float playTime;
  int currFrame;

protected:
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

  virtual bool const Cancelled() override { return _cancelled; }

  //!
  virtual void OnComplete() override
  {
    if (!_cancelled)
    {
      _callback();
    }
  }
  //!
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
  virtual void Update() override
  {
    _updater(currFrame, _totalFrames);
  }

protected:
  TFunction _updater;

};
