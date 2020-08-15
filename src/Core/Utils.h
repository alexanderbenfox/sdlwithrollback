#pragma once
#include <SDL2/SDL_rect.h>

#include <unordered_map>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>

template <typename T, typename... Rest>
bool constexpr all_base_of()
{
  return (std::is_base_of_v<T, Rest> && ...);
}

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

  static std::string CorrectPath(std::string file)
  {
#ifndef _WIN32
    auto split = StringUtils::Split(file, '\\');
    if(split.size() > 1)
      file = StringUtils::Connect(split.begin(), split.end(), '/');
#endif
    return file;
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

#include <unordered_map>
#include <list>

enum class TrieReturnValue
{
  Leaf, Branch, None
};

// real simple trie implementation for special moves
template <typename T, typename V>
class TrieNode
{
public:


  // Constructor
  TrieNode() : _endpoint(false), _value() {}
  TrieNode(std::initializer_list<std::pair<std::list<T>, V>> initList);

  void InsertSequence(const std::list<T>& sequenceKey, V value);
  TrieReturnValue Search(const std::list<T>& sequence) const;
  V const& GetKeyValue(const std::list<T>& key) const;

protected:
  // mark as friend so root can manipulate the other nodes
  friend class TrieNode<T, V>;

  const TrieNode<T, V>* Lookup(const std::list<T>& sequence) const;

  std::unordered_map<T, TrieNode<T, V>> _states;
  bool _endpoint;
  V _value;

};

template <typename T, typename V>
inline TrieNode<T, V>::TrieNode(std::initializer_list<std::pair<std::list<T>, V>> initList) : _endpoint(false)
{
  for (auto& item : initList)
    InsertSequence(item.first, item.second);
}

template <typename T, typename V>
inline void TrieNode<T, V>::InsertSequence(const std::list<T>& sequenceKey, V value)
{
  // start from root node
  TrieNode<T, V>* curr = this;
  for (auto& item : sequenceKey)
  {
    // create a new node if path doesn't exists
    if (curr->_states.find(item) == curr->_states.end())
      curr->_states.insert(std::make_pair(item, TrieNode()));

    // go to next node
    curr = &(curr->_states[item]);
  }

  // mark current node as leaf
  curr->_endpoint = true;
  // set the value
  curr->_value = value;
}

template <typename T, typename V>
inline TrieReturnValue TrieNode<T, V>::Search(const std::list<T>& sequence) const
{
  const TrieNode* item = Lookup(sequence);
  if(item == nullptr)
    return TrieReturnValue::None;
  
  // if not endpoint, it could still be a completed sequence
  return item->_endpoint ? TrieReturnValue::Leaf : TrieReturnValue::Branch;
}

template <typename T, typename V>
inline const V& TrieNode<T, V>::GetKeyValue(const std::list<T>& key) const
{
  const TrieNode* item = Lookup(key);
  if (item)
    return item->_value;
  return _value;
}

template <typename T, typename V>
inline const TrieNode<T, V>* TrieNode<T, V>::Lookup(const std::list<T>& sequence) const
{
  const TrieNode* curr = this;
  for (const auto& item : sequence)
  {
    // if not found, return nullptr
    if (curr->_states.find(item) == curr->_states.end())
      return nullptr;
    // go to next node
    curr = &((*curr->_states.find(item)).second);
  }
  return curr;
}

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

#include <functional>
//! Defer code
class ScopeGuard {
public:
  ScopeGuard() : fn_(nullptr) {}

  template<class Callable>
  ScopeGuard(Callable&& fn) : fn_(std::forward<Callable>(fn))
  {
    set = true;
  }

  ScopeGuard(ScopeGuard&& other) noexcept : fn_(std::move(other.fn_))
  {
    other.fn_ = nullptr;
  }

  ~ScopeGuard() {
    // must not throw
    if (fn_) fn_();
  }

  ScopeGuard(const ScopeGuard&) = delete;
  void operator=(const ScopeGuard&) = delete;

private:
  std::function<void()> fn_;
  bool set = false;
};

#define CONCAT_(a, b) a ## b
#define CONCAT(a, b) CONCAT_(a,b)
#define CDEFER(fn) ScopeGuard CONCAT(__defer__, __LINE__) = [&] ( ) { fn ; }

struct DeferredFn
{
  static std::list<ScopeGuard> List;
};

struct DeferScopeGuard
{
  DeferScopeGuard() = default;
  ~DeferScopeGuard() { DeferredFn::List.clear(); }
};

#define DEPAREN(X) ESC(ISH X)
#define ISH(...) ISH __VA_ARGS__
#define ESC(...) ESC_(__VA_ARGS__)
#define ESC_(...) VAN ## __VA_ARGS__
#define VANISH

#define defer(capture, code) DeferredFn::List.emplace_back(CONCAT([DEPAREN(capture)](), { code ; }));

//#include "common.h"
//#define defer(code) KJ_DEFER(code)