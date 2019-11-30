typedef std::function<void(int)> UpdateFunction;
typedef std::function<void()> FrameFunction;

class Timer
{
public:
  Timer() : _frames(0.0f)
  {
    _cpuClock.SetFPS();
  }

  void Start()
  {
    _lastFrameTime(_cpuClock.now());
    _sdlClock.Start();
  }

  void Update(
    FrameFunction& updateInput,
    StateFunction& updateFunction,
    FrameFunction& drawFunction)
  {
    _frames++;

    uint32_t dt = _cpuClock.now() - _lastFrameTime;
    _lastFrameTime = _cpuClock.now();

    _cpuClock.lag += dt;

    updateInput();

    if(fixedTimeStep)
    {
      //try to maintain 60 fps
      while(_cpuClock.lag >= _cpuClock.timestep)
      {
        _cpuClock.lag -= _cpuClock.timestep;
        updateFunction(_cpuClock.frametime);
      }
    }
    else
    {
      updateFunction(_dt);
    }

    drawFunction();

    if(fixedTimeStep)
    {
      //cap framerate
      if(_sdlClock.GetTicks() < _cpuClock.timestep)
      {
        SDL_Delay(_cpuClock.timestep - _sdlClock.GetTicks());
      }
    }
  }

private:

  float _frames;
  uint32_t _lastFrameTime;
  bool fixedTimeStep;

  struct IClock
  {
    virtual IClock() = 0;
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual void Pause() = 0;

    virtual int GetTicks() = 0;
  }

  struct SDLClock : public IClock
  {
    SDLClock() : startTicks(0), pausedTicks(0), paused(false), started(false) {}
    int startTicks, pauseTicks;
    bool paused, started;

    void Start() override
    {
      if(!started)
      {
        started = true;
        paused = false;
        startTicks = SDL_GetTicks();
      }
      else if (paused)
      {
        paused = false;
        startTicks = SDL_GetTicks() - pausedTicks;
        pauseTicks = 0;
      }
    }

    void Stop() override
    {
      started = false;
      paused = false;
    }

    void Pause() override
    {
      if(started && !paused)
      {
        paused = true;
        pauseTicks = SDL_GetTicks() - startTicks;
      }
    }

    int GetTicks() override
    {
      if(started)
      {
        //return value when the pause began
        if(paused)
          return pauseTicks;
        else
        return SDL_GetTicks() - startTicks;
      }
    }
    return 0;
  }

  struct CPUClock
  {
    typedef std::chrono::milliseconds ms;
    typedef std::chrono::nanoseconds ns;

    uint32_t lag;
    int fps;

    int timestep = 1000/fps;
    float frametime = 0.0075*(30.0/(float)fps);

    static uint32_t now() noexcept
    {
      return SDL_GetTicks();
    }

    void SetFPS()
    {
      SDL_DisplayMode mode = {SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0};
      SDL_GetDisplayMode(0, 0, &mode);
      fps = mode.refresh_rate;
    }
  }

  SDLClock _sdlClock;
  CPUClock _cpuClock;


}