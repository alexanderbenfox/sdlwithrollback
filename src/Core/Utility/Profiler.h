#pragma once
// based on https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Debug/Instrumentor.h
// output can be loaded in chrome://tracing/ for a visual profile

#include <chrono>
#include <string>
#include <thread>
#include <mutex>
#include <fstream>


#include <algorithm>
#include <iomanip>
#include <sstream>

#include "Core/Utility/String.h"

class Profiler
{
public:

  //! regular time in microseconds used for profiling
  using Time = std::chrono::microseconds;
  //! floating point precision time type
  using FTime = std::chrono::duration<double, std::micro>;

  struct Result
  {
    std::string name;
    FTime start;
    Time elapsed;
    std::thread::id tID;
  };

  struct Session
  {
    std::string name;
  };

  class Timer
  {
  public:
    Timer(const char* name) : _name(name), _stopped(false)
    {
      _start = std::chrono::steady_clock::now();
    }

    ~Timer()
    {
      if (!_stopped)
        Stop();
    }

    void Stop()
    {
      auto endTimepoint = std::chrono::steady_clock::now();
      auto highResStart = FTime{ _start.time_since_epoch() };
      auto elapsedTime = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch() - std::chrono::time_point_cast<std::chrono::microseconds>(_start).time_since_epoch();

      Profiler::Get().WriteProfile({ _name, highResStart, elapsedTime, std::this_thread::get_id() });

      _stopped = true;
    }

  private:
    const char* _name;
    std::chrono::time_point<std::chrono::steady_clock> _start;
    bool _stopped;

  };

  static Profiler& Get()
  {
    static Profiler instance;
    return instance;
  }

  void BeginSession(const std::string_view name, const std::string_view relativePath)
  {
    std::lock_guard lock(_mutex);
    if (_currSession)
    {
      InternalEndSession();
    }
    
    _outputStream.open(StringUtils::CorrectPath(relativePath.data()));
    if (_outputStream.is_open())
    {
      _currSession = new Session{ name.data() };
      WriteHeader();
    }
    else
    {
      throw std::exception("Profiler could not open/create results file.\n");
    }

  }

  void EndSession()
  {
    std::lock_guard lock(_mutex);
    InternalEndSession();
  }

  void WriteProfile(const Result& result)
  {
    std::stringstream json;

    std::string name = result.name;
    std::replace(name.begin(), name.end(), '"', '\'');

    json << std::setprecision(3) << std::fixed;
    json << ",{";
    json << "\"cat\":\"function\",";
    json << "\"dur\":" << (result.elapsed.count()) << ',';
    json << "\"name\":\"" << name << "\",";
    json << "\"ph\":\"X\",";
    json << "\"pid\":0,";
    json << "\"tid\":" << result.tID << ",";
    json << "\"ts\":" << result.start.count();
    json << "}";

    std::lock_guard lock(_mutex);
    if (_currSession)
    {
      _outputStream << json.str();
      _outputStream.flush();
    }
  }

  template <size_t N>
  struct ChangeResult
  {
    char Data[N];
  };

  template <size_t N, size_t K>
  static constexpr auto CleanupOutputString(const char(&expr)[N], const char(&remove)[K])
  {
    ChangeResult<N> result = {};

    size_t srcIndex = 0;
    size_t dstIndex = 0;
    while (srcIndex < N)
    {
      size_t matchIndex = 0;
      while (matchIndex < K - 1 && srcIndex + matchIndex < N - 1 && expr[srcIndex + matchIndex] == remove[matchIndex])
        matchIndex++;
      if (matchIndex == K - 1)
        srcIndex += matchIndex;
      result.Data[dstIndex++] = expr[srcIndex] == '"' ? '\'' : expr[srcIndex];
      srcIndex++;
    }
    return result;
  }

private:
  Profiler() : _currSession(nullptr) {}

  void WriteHeader()
  {
    _outputStream << "{\"otherData\": {},\"traceEvents\":[{}";
    _outputStream.flush();
  }

  void WriteFooter()
  {
    _outputStream << "]}";
    _outputStream.flush();
  }

  void InternalEndSession()
  {
    if (_currSession)
    {
      WriteFooter();
      _outputStream.close();
      delete _currSession;
      _currSession = nullptr;
    }
  }

  std::mutex _mutex;
  Session* _currSession;
  std::ofstream _outputStream;

};

// set to 0 when not on
#define PROFILE_ON 1

#if PROFILE_ON

  // Resolve which function signature macro will be used. Note that this only
  // is resolved when the (pre)compiler starts, so the syntax highlighting
  // could mark the wrong one in your editor!
  #if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
  #define FUNC_SIG __PRETTY_FUNCTION__
  #elif defined(__DMC__) && (__DMC__ >= 0x810)
  #define FUNC_SIG  __PRETTY_FUNCTION__
  #elif defined(__FUNCSIG__)
  #define FUNC_SIG  __FUNCSIG__
  #elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
  #define FUNC_SIG  __FUNCTION__
  #elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
  #define FUNC_SIG  __FUNC__
  #elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
  #define FUNC_SIG  __func__
  #elif defined(__cplusplus) && (__cplusplus >= 201103)
  #define FUNC_SIG  __func__
  #else
  #define FUNC_SIG  "FUNC_SIG unknown!"
  #endif

  #define PROFILE_BEGIN_SESSION(name, filepath) Profiler::Get().BeginSession(name, filepath)
  #define PROFILE_END_SESSION() Profiler::Get().EndSession()
  #define PROFILE_SCOPE_LINE2(name, line) constexpr auto fixedName##line = Profiler::CleanupOutputString(name, "__cdecl ");\
											     Profiler::Timer timer##line(fixedName##line.Data)
  #define PROFILE_SCOPE_LINE(name, line) PROFILE_SCOPE_LINE2(name, line)
  #define PROFILE_SCOPE(name) PROFILE_SCOPE_LINE(name, __LINE__)
  #define PROFILE_FUNCTION() PROFILE_SCOPE(FUNC_SIG)

#else
  #define PROFILE_BEGIN_SESSION(name, filepath)
  #define PROFILE_END_SESSION()
  #define PROFILE_SCOPE(name)
  #define PROFILE_FUNCTION()

#endif
