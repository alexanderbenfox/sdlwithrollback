#pragma once

//! Two-way map used for configuration maps like gamepad inputs
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

    //! Sets value on the map
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

    //! Getters
    operator T1 const& () { return key; }
    operator T2 const& () { return value; }
  };

public:
  ConfigMap() = default;

  //! Sets key and value for two way map
  void SetValue(T1 a, T2 b);

  //! override getter operators
  T2 const& operator[](const T1& key) const;
  T1 const& operator[](const T2& key) const;
  ValueWrapper operator[](const T2& key) { return ValueWrapper(_backward[key], key, this); }
  ValueWrapper operator[](const T1& key) { return ValueWrapper(key, _forward[key], this); }

  std::vector<T1> const& GetKeys() { return _keys; }

private:
  std::unordered_map<T1, T2> _forward;
  std::unordered_map<T2, T1> _backward;
  std::vector<T1> _keys;

};

template <typename T1, typename T2>
void ConfigMap<T1, T2>::SetValue(T1 a, T2 b)
{
  //so that multiple keys don't map to the same value
  if (_backward.find(b) != _backward.end())
  {
    T1 currentKey = _backward[b];
    _forward.erase(currentKey);
    int i;
    for (i = 0; i < _keys.size(); i++)
    {
      if (_keys[i] == currentKey)
        break;
    }
    _keys.erase(_keys.begin() + i);
  }

  _forward[a] = b;
  _backward[b] = a;
  _keys.push_back(a);
}

template <typename T1, typename T2>
inline T2 const& ConfigMap<T1, T2>::operator[](const T1& key) const
{
  return _forward[key];
}

template <typename T1, typename T2>
inline T1 const& ConfigMap<T1, T2>::operator[](const T2& key) const
{
  return _backward[key];
}
