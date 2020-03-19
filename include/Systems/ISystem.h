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

static const uint64_t motion_id = 1 << 0;
static const uint64_t sprite_id = 1 << 1;
static const uint64_t sound_id = 1 << 2;
static const uint64_t particle_id = 1 << 3;

// Signature to check for entities with motion, sprite, and 
// particle components.
static const uint64_t sig = motion_id | sprite_id | particle_id;

template <typename ... T>
class ISystem
{
public:
  //static void DoTick(float dt) {}

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
