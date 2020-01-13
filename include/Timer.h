#include <functional>

typedef std::function<void(float)> UpdateFunction;

class Timer
{
public:
  Timer() : _frames(0.0f), fixedTimeStep(true)
  {
    _sdlClock.SetFPS();
  }

  void Start()
  {
    _lastFrameTime = _sdlClock.now();
    _sdlClock.Start();
  }

  void Update(UpdateFunction& updateFunction)
  {
    uint32_t dt = _sdlClock.now() - _lastFrameTime;
    _lastFrameTime = _sdlClock.now();

    _sdlClock.lag += dt;

    //updateInput();

    if(fixedTimeStep)
    {
      //try to maintain 60 fps
      while(_sdlClock.lag >= _sdlClock.timestep)
      {
        _frames++;
        _sdlClock.lag -= _sdlClock.timestep;
        updateFunction(_sdlClock.frametime);
      }
    }
    else
    {
      float dtS = static_cast<float>(dt) / 1000.0f;
      updateFunction(dtS);
    }

    //drawFunction();

    if(fixedTimeStep)
    {
      //cap framerate
      if(_sdlClock.GetElapsedTicks() < _sdlClock.timestep)
      {
        SDL_Delay(_sdlClock.timestep - _sdlClock.GetElapsedTicks());
      }
    }
  }

private:

  float _frames;
  uint32_t _lastFrameTime;
  bool fixedTimeStep;

  struct IClock
  {
    //virtual ~IClock() = 0;
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual void Pause() = 0;

    virtual uint32_t GetElapsedTicks() const = 0;
  };

  struct SDLClock : public IClock
  {
    SDLClock() : startTicks(0), pauseTicks(0), lag(0), paused(false), started(false) {}
    uint32_t startTicks, pauseTicks;
    uint32_t lag;
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
        startTicks = SDL_GetTicks() - pauseTicks;
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

    uint32_t GetElapsedTicks() const override
    {
      if(started)
      {
        //return value when the pause began
        if(paused)
          return pauseTicks;
        else
        return SDL_GetTicks() - startTicks;
      }
      return 0;
    }

    uint32_t fps;
    uint32_t timestep;
    float frametime;

    static uint32_t now() noexcept
    {
      return SDL_GetTicks();
    }

    void SetFPS()
    {
      SDL_DisplayMode mode = { SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0 };
      SDL_GetDisplayMode(0, 0, &mode);
      fps = mode.refresh_rate;
      timestep = 1000 / fps;
      frametime = 1.0f / (float)fps;//0.0075f*(30.0f / (float)fps);
    }
  };



  SDLClock _sdlClock;

};
