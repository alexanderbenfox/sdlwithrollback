#include <functional>
#include <vector>
#include <SDL2/SDL_timer.h>

typedef std::function<void(float)> UpdateFunction;

class AvgCounter
{
public:
  void Add(long long value)
  {
    const int totalSize = 200;

    _total += value;
    if(_updateCount >= (totalSize - 1) || _filled)
    {
      _filled = true;
      _total -= _lru[_updateCount % totalSize];
    }
    _lru[_updateCount % totalSize] = value;

    _updateCount = (++_updateCount) % totalSize;
  }

  long long Count()
  {
    return _filled ? _total / (long long)200 : _updateCount == 0 ? 0 : _total / (long long)_updateCount;
  }
private:
  int _updateCount = 0;
  long long _total;
  long long _lru[200];
  bool _filled = false;

};

// is there any point to this being an interface...
struct IClock
{
  //virtual ~IClock() = 0;
  virtual void Start() = 0;
  virtual void Stop() = 0;
  virtual void Pause() = 0;

  virtual uint32_t GetElapsedTicks() const = 0;
};

// Want to fix this class eventually
struct SDLClock : public IClock
{
  SDLClock();

  void Start() override;

  void Stop() override;

  void Pause() override;

  uint32_t GetElapsedTicks() const override;

  float GetElapsedTimeSeconds() const;

  static uint32_t now() noexcept { return SDL_GetTicks(); }

  void SetFPS();

  uint32_t startTicks, pauseTicks;
  uint32_t lag;
  bool paused, started;
  uint32_t fps;
  uint32_t timestep;
  float frametime;
};

//! Time manager class
class Timer
{
public:
  //! Constructor
  Timer() : _frames(0.0f), _fixedTimeStep(true), _lastFrameTime(_mainClock.now()) {}
  //! Start function gets the FPS from internal hardward
  void Start();
  //! Begins pause coroutine on this clock for the given time
  void PauseForTime(float seconds);
  //! Begins a user custom coroutine function
  void BeginCoroutine(float seconds, UpdateFunction function);
  //! Updates timer and runs the user specified update function
  void Update(UpdateFunction& updateFunction);
  //! Gets average update time in nanoseconds
  long long GetUpdateTime() { return _perfCounter.Count(); }

private:
  //! coroutine class 
  class Coroutine
  {
  public:
    enum class Status
    {
      NotStarted, Running, Complete
    };
    //! Intended to be constructed in place - timer is started on construction
    Coroutine(float time, UpdateFunction update);
    //! Update function should run once before the coroutine is removed
    Status Update() const;

  private:
    //! User assigned update function
    UpdateFunction _update;
    //! Total time the coroutine should run for
    float _totalTime;
    //! Internal coroutine clock
    SDLClock _clock;
  };

  //! Updates all coroutines
  void UpdateCoroutines();

  //! All frames elapsed
  float _frames;
  //! Time of last update frame
  uint32_t _lastFrameTime;
  //! Flags whether or not this timer runs on a fixed time step
  bool _fixedTimeStep;
  //! Running coroutines
  std::vector<Coroutine> _coroutines;
  //! Function for removing coroutine from list as it updates
  const std::function<bool(const Coroutine&)> _coroutineUpdate = [](const Coroutine& coroutine)
  {
    return coroutine.Update() == Coroutine::Status::Complete;
  };
  //! Main clock
  SDLClock _mainClock;
  //!
  AvgCounter _perfCounter;

};
