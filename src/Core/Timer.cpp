#include "Timer.h"
#include <SDL2/SDL.h>
#include <algorithm>
#include <chrono>

#include "Managers/GGPOManager.h"

//______________________________________________________________________________
SDLClock::SDLClock() : startTicks(0), pauseTicks(0), lag(0), paused(false), started(false) {}

//______________________________________________________________________________
void SDLClock::Start()
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

//______________________________________________________________________________
void SDLClock::Stop()
{
  started = false;
  paused = false;
}

//______________________________________________________________________________
void SDLClock::Pause()
{
  if(started && !paused)
  {
    paused = true;
    pauseTicks = SDL_GetTicks() - startTicks;
  }
}

//______________________________________________________________________________
uint32_t SDLClock::GetElapsedTicks() const
{
  if(started)
  {
    //return value when the pause began
    if(paused)
      return pauseTicks;
    return SDL_GetTicks() - startTicks;
  }
  return 0;
}

//______________________________________________________________________________
float SDLClock::GetElapsedTimeSeconds() const
{
  return static_cast<float>(GetElapsedTicks()) / 1000.0f;
}

//______________________________________________________________________________
void SDLClock::SetFPS()
{
  SDL_DisplayMode mode = { SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0 };
  SDL_GetDisplayMode(0, 0, &mode);
  fps = mode.refresh_rate;
  timestep = 1000 / fps;
  frametime = 1.0f / (float)fps;//0.0075f*(30.0f / (float)fps);
}

//______________________________________________________________________________
Timer::Coroutine::Coroutine(float time, UpdateFunction update) : _totalTime(time), _update(update)
{
  _clock.SetFPS();
  _clock.Start();
}

//______________________________________________________________________________
Timer::Coroutine::Status Timer::Coroutine::Update() const
{
  _update(_clock.GetElapsedTimeSeconds() / _totalTime);

  if(_clock.GetElapsedTimeSeconds() == 0.0f)
    return Status::NotStarted;

  if(_clock.GetElapsedTimeSeconds() / _totalTime >= 1.0f)
    return Status::Complete;
  return Status::Running;
}

//______________________________________________________________________________
void Timer::Start(int fps)
{
  _renderClock.SetFPS();

  _mainClock.fps = fps;
  _mainClock.timestep = 1000.f / static_cast<float>(fps);
  _mainClock.frametime = 1.0f / static_cast<float>(fps);
  _lastFrameTime = _mainClock.GetElapsedTicks();
  _mainClock.Start();
}

//______________________________________________________________________________
void Timer::PauseForTime(float seconds)
{
  auto pauseFunction = [this](float percentComplete)
  {
    if(percentComplete >= 1.0f)
    {
      _mainClock.Start();
    }
  };

  // do the "On Start" function before sending it to the coroutine list 
  _mainClock.Pause();
  
  BeginCoroutine(seconds, pauseFunction);
}

//______________________________________________________________________________
void Timer::BeginCoroutine(float seconds, UpdateFunction function)
{
  // construct the event in place
  _coroutines.emplace_back(seconds, function);
}

//______________________________________________________________________________
void Timer::Update(UpdateFunction& updateFunction)
{
  // update all of the coroutines
  UpdateCoroutines();

  uint32_t dt = _mainClock.GetElapsedTicks() - _lastFrameTime;
  _lastFrameTime = _mainClock.GetElapsedTicks();
  _mainClock.lag += dt;

  if(_fixedTimeStep)
  {
    if (_mainClock.paused)
    {
      std::chrono::time_point<std::chrono::high_resolution_clock> tp1 = std::chrono::steady_clock::now();
      updateFunction(0);

      long long timeToUpdate = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - tp1).count();
      _perfCounter.Add(timeToUpdate);
    }

    //try to maintain 60 fps
    while (_mainClock.lag >= _mainClock.timestep)
    {
      
      _frames++;
      _mainClock.lag -= _mainClock.timestep;

      std::chrono::time_point<std::chrono::high_resolution_clock> tp1 = std::chrono::steady_clock::now();
      updateFunction(_mainClock.frametime);

      long long timeToUpdate = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - tp1).count();
      _perfCounter.Add(timeToUpdate);
    } 

    //cap framerate
    uint32_t updatedDt = (_mainClock.GetElapsedTicks() - _lastFrameTime) + dt;
    uint32_t delayMS = _mainClock.timestep - updatedDt % _mainClock.timestep;

#ifdef _WIN32
    if (GGPOManager::Get().InMatch())
      GGPOManager::Get().Idle(delayMS);
    else
      SDL_Delay(delayMS);
#else
    SDL_Delay(delayMS);
#endif
    
  }
  else
  {
    float dtS = static_cast<float>(dt) / 1000.0f;
    updateFunction(dtS);
  }
}

//______________________________________________________________________________
void Timer::UpdateCoroutines()
{
  auto end = std::remove_if(_coroutines.begin(), _coroutines.end(), _coroutineUpdate);
  _coroutines.erase(end, _coroutines.end());
}
