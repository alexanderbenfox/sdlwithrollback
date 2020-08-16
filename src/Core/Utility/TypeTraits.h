#pragma once

template <typename T, typename... Rest>
bool constexpr all_base_of()
{
  return (std::is_base_of_v<T, Rest> && ...);
}

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
