#pragma once
#include "Core/ECS/IComponent.h"
#include <array>

class Entity;

class IComponentArray
{
public:
  virtual ~IComponentArray() = default;
};

template<typename T = IComponent>
class ComponentArray : public IComponentArray
{
public:
  static ComponentArray<T>& Get()
  {
    static ComponentArray<T> array;
    return array;
  }
  //! Insert new component data into index
  void Insert(EntityID id, T component);
  //! Removes component data and disassociates it from entity
  void Remove(EntityID entity);
  //! Checks if entity ID is associated with data in array
  bool HasComponent(EntityID entity) { return _entityToIndexMap.find(entity) != _entityToIndexMap.end(); }
  //! Gets component data for entity from array
  T& GetComponent(EntityID entity);
  //! Runs function on each living component data in the array
  void ForEach(std::function<void(T&)> fn);

private:
  //! default initialization
  ComponentArray() = default;
  ComponentArray(const ComponentArray&) = delete;
  ComponentArray(ComponentArray&&) = delete;
  ComponentArray operator=(const ComponentArray&) = delete;
  ComponentArray operator=(ComponentArray&&) = delete;

  //! packed array of components (this will maintain the density in the array)
  std::array<T, MAX_ENTITIES> _components;
  //! Entity id to index in array map
  std::unordered_map<EntityID, uint32_t> _entityToIndexMap;
  //! Map from an array index to entity ID
  std::unordered_map<uint32_t, EntityID> _indexToEntityMap;
  //! Total size of valid entries in array
  uint32_t _size = 0;

};

template <typename T>
inline void ComponentArray<T>::Insert(EntityID id, T component)
{
  if (!HasComponent(id))
  {
    uint32_t newIndex = _size;

    _entityToIndexMap[id] = newIndex;
    _indexToEntityMap[newIndex] = id;
    _components[newIndex] = component;
    _components[newIndex].OnAdd(id);
    _size++;
  }
}

template <typename T>
inline void ComponentArray<T>::Remove(EntityID entity)
{
  if (HasComponent(entity))
  {
    // swap element at end into deleted element's place to maintain density
    uint32_t removedIndex = _entityToIndexMap[entity];
    // get last element
    uint32_t indexLastElement = _size - 1;
    // force old component to be deleted
    _components[removedIndex].OnRemove(entity);
    _components[removedIndex] = std::move(_components[indexLastElement]);

    // update map to point to the moved spot
    EntityID entityLastElemet = _indexToEntityMap[indexLastElement];
    _entityToIndexMap[entityLastElemet] = removedIndex;
    _indexToEntityMap[removedIndex] = entityLastElemet;

    // finally erase entries
    _entityToIndexMap.erase(entity);
    _indexToEntityMap.erase(indexLastElement);

    --_size;
  }
}

template <typename T>
inline T& ComponentArray<T>::GetComponent(EntityID entity)
{
  return _components[_entityToIndexMap[entity]];
}

template <typename T>
inline void ComponentArray<T>::ForEach(std::function<void(T&)> fn)
{
  for (uint32_t i = 0; i < _size; i++)
  {
    fn(_components[i]);
  }
}
