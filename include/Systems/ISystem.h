#pragma once
#include <cstdint>
#include "Entity.h"
#include <tuple>
#include <map>

template <typename ... T>
struct Requires {};

template <typename T>
struct Requires<T>
{
  static bool MatchesSignature(Entity* entity)
  {
    return (entity->ComponentBitFlag & ComponentTraits<T>::GetSignature()) == ComponentTraits<T>::GetSignature();
  }
};

template <typename T, typename ... Rest>
struct Requires<T, Rest...>
{
  static bool MatchesSignature(Entity* entity)
  {
    auto combinedSignature = ComponentTraits<T>::GetSignature() | (ComponentTraits<Rest>::GetSignature() | ...);
    return (entity->ComponentBitFlag & combinedSignature) == combinedSignature;
  }

  static bool HasOneRequirement(Entity* entity)
  {
    auto combinedSignature = ComponentTraits<T>::GetSignature() | (ComponentTraits<Rest>::GetSignature() | ...);
    return (entity->ComponentBitFlag & combinedSignature) != 0;
  }
};

template <typename ... T>
class ISystem
{
public:
  //! 
  static void Check(Entity* entity)
  {
    if(Req::MatchesSignature(entity))
    {
      auto it = Tuples.find(entity->GetID());
      if (it == Tuples.end())
      {
        auto things = std::make_tuple((entity->GetComponent<T>(), ...));
        std::tuple<std::add_pointer_t<T>...> tuple = entity->MakeComponentTuple<T...>();
        Tuples.insert(std::make_pair(entity->GetID(), tuple));
      }
      else
      {
        Tuples[entity->GetID()] = entity->MakeComponentTuple<T...>();
      }
    }
    else
    {
      auto it = Tuples.find(entity->GetID());
      if(it != Tuples.end())
        Tuples.erase(it);
    }
  }

  //!
  static std::map<int, std::tuple<std::add_pointer_t<T>...>> Tuples;

protected:
  //! Required components
  using Req = Requires<T...>;

};

template <typename ... T>
std::map<int, std::tuple<std::add_pointer_t<T>...>> ISystem<T...>::Tuples;


//_________________________________________________________________________
// Multi System facilitates interaction between multiple entities in a system

template <typename ... T>
struct SysComponents {};

template <typename Main, typename Sub>
class IMultiSystem;

template <typename ... Main, typename ... Sub>
class IMultiSystem<SysComponents<Main...>, SysComponents<Sub...>>
{
public:
  class MainSystem : public ISystem<Main...> {};

  class SubSystem : public ISystem<Sub...> {};

  static void Check(Entity* entity)
  {
    MainSystem::Check(entity);
    SubSystem::Check(entity);
  }
};
