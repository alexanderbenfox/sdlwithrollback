#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <iterator>

namespace StringUtils
{
  template <typename Out>
  static void Split(const std::string &s, char delim, Out result)
  {
    std::istringstream iss(s);
    std::string item;
    while (std::getline(iss, item, delim)) {
      *result++ = item;
    }
  }

  static std::vector<std::string> Split(const std::string &s, char delim)
  {
    std::vector<std::string> elems;
    Split(s, delim, std::back_inserter(elems));
    return elems;
  }

  static std::string Connect(std::vector<std::string>::iterator const& beg, std::vector<std::string>::iterator const& end, char delim)
  {
    std::stringstream ss("");
    auto emit = [&ss, delim, need_delim = false](std::string const& x) mutable
    {
      if (need_delim) ss << delim;
      ss << x;
      need_delim = true;
    };

    for (auto it = beg; it != end; ++it)
      emit(*it);
    return ss.str();
  }
}

namespace OpSysConv
{
  static SDL_Rect CreateSDLRect(const int& x, const int& y, const int& width, const int& height)
  {
#ifdef _WIN32
    return SDL_Rect{ x, y, width, height };
#else
    return SDL_Rect{ .x = x,.y = y,.w = width,.h = height };
#endif
  }
}

template <typename T1, typename T2>
class ConfigMap
{
private:

  struct ValueWrapper
  {
    ValueWrapper(T1 k, T2 v, ConfigMap* m) : key(k), value(v), map(m) {}
    T1 key;
    T2 value;
    ConfigMap* map;

    ValueWrapper& operator=(T1 const& rhs)
    {
      map->SetValue(rhs, value);
      return *this;
    }

    ValueWrapper& operator=(T2 const& rhs)
    {
      map->SetValue(key, rhs);
      return *this;
    }

    operator T1 const&()
    {
      return key;
    }

    operator T2 const&()
    {
      return value;
    }
  };

public:
  ConfigMap() = default;

  void SetValue(T1 a, T2 b)
  {
    //so that multiple keys don't map to the same value
    if (_backward.find(b) != _backward.end())
    {
      T1 currentKey = _backward[b];
      _forward.erase(currentKey);
    }

    _forward[a] = b;
    _backward[b] = a;
  }

  T2 const& operator[](const T1& key) const
  {
    return _forward[key];
  }

  T1 const& operator[](const T2& key) const
  {
    return _backward[key];
  }

  ValueWrapper operator[](const T2& key)
  {
    return ValueWrapper(_backward[key], key, this);
  }

  ValueWrapper operator[](const T1& key)
  {
    return ValueWrapper(key, _forward[key], this);
  }

private:

  std::unordered_map<T1, T2> _forward;
  std::unordered_map<T2, T1> _backward;
};


// trick for having multiple inheritence with shared from this. Found: https://stackoverflow.com/questions/16082785/use-of-enable-shared-from-this-with-multiple-inheritance
/* Trick to allow multiple inheritance of objects
 * inheriting shared_from_this.
 * cf. https://stackoverflow.com/a/12793989/587407
 */

 /* First a common base class
  * of course, one should always virtually inherit from it.
  */

class MultipleInheritableEnableSharedFromThis : public std::enable_shared_from_this<MultipleInheritableEnableSharedFromThis>
{
public:
  virtual ~MultipleInheritableEnableSharedFromThis()
  {}
};


template <class T>
class inheritable_enable_shared_from_this : virtual public MultipleInheritableEnableSharedFromThis
{
public:
  std::shared_ptr<T> shared_from_this() {
    return std::dynamic_pointer_cast<T>(MultipleInheritableEnableSharedFromThis::shared_from_this());
  }
  /* Utility method to easily downcast.
   * Useful when a child doesn't inherit directly from enable_shared_from_this
   * but wants to use the feature.
   */
  template <class Down>
  std::shared_ptr<Down> downcasted_shared_from_this() {
    return std::dynamic_pointer_cast<Down>(MultipleInheritableEnableSharedFromThis::shared_from_this());
  }
};
