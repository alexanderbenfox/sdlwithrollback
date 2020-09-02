#include "Systems/TimerSystem/ActionTimer.h"

void ActionTimer::Serialize(std::ostream& os) const
{
  os << playTime;
  os << currFrame;
  os << _totalFrames;
}

void ActionTimer::Deserialize(std::istream& is)
{
  is >> playTime;
  is >> currFrame;
  is >> _totalFrames;
}

void SimpleActionTimer::OnComplete()
{
  if (!_cancelled)
    _callback();
}

void ComplexActionTimer::Update()
{
  if(!_cancelled)
    _updater(currFrame, _totalFrames);
}
