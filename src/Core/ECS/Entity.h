#pragma once
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <functional>

#include "Core/ECS/ComponentArray.h"
#include "Core/ECS/EntityManager.h"

#include "Core/Math/Vector2.h"
#include "Core/Utils.h"

//______________________________________________________________________________
//! Entity is a wrapper for the connection between the entity manager and the component manager(s)
class Entity : public std::enable_shared_from_this<Entity>
{
public:
  //! Register new entity to the entity manager
  Entity();
  //! Set signature to null, reset systems, deregister from entity manager
  ~Entity();

  //! Removes all added components by calling list of deleter functions
  void RemoveAllComponents();
  //! Requests the game manager destroys this entity at the end of the frame
  void DestroySelf();
  //! Sets scale among all scalable components attached to this... THERE MUST BE A BETTER WAY
  void SetScale(Vector2<float> scale);
  //! Gets a unique id for the entity
  int GetID() const { return _id; }
  //! Get component signature for systems
  ComponentBitFlag GetSignature() { return EntityManager::Get().GetSignature(_id); }

  //! Retrieves the components of type specified or nullptr if there is no component of that type present
  template <typename T = IComponent> 
  T* GetComponent();
  //! Adds the component of the type specified to this entity
  template <typename T = IComponent>
  void AddComponent();
  //! Adds the component with initialization parameters
  template <typename T = IComponent>
  void AddComponent(const ComponentInitParams<T>& initParams);
  //! Multi-parameter component add
  template <typename T = IComponent, typename ... Rest>
  void AddComponents();
  //! Removes component of type specified from the entity
  template <typename T = IComponent>
  void RemoveComponent();
  //! Multi-parameter component remove
  template <typename T = IComponent, typename ... Rest>
  void RemoveComponents();

protected:
  //! Don't remove deletion function here. This is used for removing components internally if we do not know the type
  template <typename T = IComponent>
  void RemoveComponentInternal();
  //! Add component of type specified type to the entity without calling check against systems
  template <typename T = IComponent>
  void AddComponentNoSystemCheck();
  //! Removes component of type specified type from the entity without calling check against systems
  template <typename T = IComponent>
  void RemoveComponentNoSystemCheck();

  //! Checks against all existing systems (currently hardcoded in the game manager)
  static void CheckAgainstSystems(Entity* entity);


  //! Protected Members

  //! Functors for deleting components en masse (without knowing type at runtime) uses RemoveComponentInternal
  std::unordered_map<std::type_index, std::function<void()>> _deleteComponent;
  //! This entity ID (and order of creation)
  EntityID _id;

};

//______________________________________________________________________________
template <typename T>
inline T* Entity::GetComponent()
{
  if(ComponentArray<T>::Get().HasComponent(_id))
    return &ComponentArray<T>::Get().GetComponent(_id);
  return nullptr;
}

//______________________________________________________________________________
template <typename T>
inline void Entity::AddComponent()
{
  if (!ComponentArray<T>::Get().HasComponent(_id))
  {
    ComponentArray<T>::Get().Insert(_id, T());

    auto signature = GetSignature();
    signature |= ComponentTraits<T>::GetSignature();
    EntityManager::Get().SetSignature(_id, signature);

    // add to special deleter list
    _deleteComponent.insert(std::make_pair(std::type_index(typeid(T)), [this]() { RemoveComponentInternal<T>(); }));

    // see if this needs to be added to the system
    CheckAgainstSystems(this);
  }
}

//______________________________________________________________________________
template <typename T>
inline void Entity::AddComponent(const ComponentInitParams<T>& initParams)
{
  if (!ComponentArray<T>::Get().HasComponent(_id))
  {
    ComponentArray<T>::Get().Insert(_id, T());
    ComponentInitParams<T>::Init(ComponentArray<T>::Get().GetComponent(_id), initParams);

    auto signature = GetSignature();
    signature |= ComponentTraits<T>::GetSignature();
    EntityManager::Get().SetSignature(_id, signature);

    // add to special deleter list
    _deleteComponent.insert(std::make_pair(std::type_index(typeid(T)), [this]() { RemoveComponentInternal<T>(); }));

    // see if this needs to be added to the system
    CheckAgainstSystems(this);
  }
  else
  {
    // if already exists in component array, reinit
    ComponentInitParams<T>::Init(*GetComponent<T>(), initParams);
  }
}

//______________________________________________________________________________
template <typename T, typename ... Rest>
inline void Entity::AddComponents()
{
  // recursive control path enders
  if (!all_base_of<IComponent, T, Rest...>() || std::is_same_v<T, IComponent>)
    return CheckAgainstSystems(this);

  AddComponentNoSystemCheck<T>();
  AddComponents<Rest...>();
}

//______________________________________________________________________________
template <typename T>
inline void Entity::RemoveComponent()
{
  if (ComponentArray<T>::Get().HasComponent(_id))
  {
    ComponentArray<T>::Get().Remove(_id);

    auto signature = GetSignature();
    signature &= ~ComponentTraits<T>::GetSignature();
    EntityManager::Get().SetSignature(_id, signature);

    // remove from special deleter list
    _deleteComponent.erase(std::type_index(typeid(T)));

    CheckAgainstSystems(this);
  }
}

//______________________________________________________________________________
template <typename T, typename ... Rest>
inline void Entity::RemoveComponents()
{
  // recursive control path ender will check the system 
  if (!all_base_of<IComponent, T, Rest...>() || std::is_same_v<T, IComponent>)
    return CheckAgainstSystems(this);

  // use no system check here so it can be a little more efficient
  RemoveComponentNoSystemCheck<T>();
  RemoveComponents<Rest...>();
}

//______________________________________________________________________________
template <typename T>
inline void Entity::RemoveComponentInternal()
{
  if (ComponentArray<T>::Get().HasComponent(_id))
  {
    ComponentArray<T>::Get().Remove(_id);

    auto signature = GetSignature();
    signature &= ~ComponentTraits<T>::GetSignature();
    EntityManager::Get().SetSignature(_id, signature);
  }
}

//______________________________________________________________________________
template <typename T>
inline void Entity::AddComponentNoSystemCheck()
{
  if (!ComponentArray<T>::Get().HasComponent(_id))
  {
    ComponentArray<T>::Get().Insert(_id, T());

    auto signature = GetSignature();
    signature |= ComponentTraits<T>::GetSignature();
    EntityManager::Get().SetSignature(_id, signature);

    // add to special deleter list
    _deleteComponent.insert(std::make_pair(std::type_index(typeid(T)), [this]() { RemoveComponentInternal<T>(); }));
  }
}

//______________________________________________________________________________
template <typename T>
inline void Entity::RemoveComponentNoSystemCheck()
{
  if (ComponentArray<T>::Get().HasComponent(_id))
  {
    ComponentArray<T>::Get().Remove(_id);

    auto signature = GetSignature();
    signature &= ~ComponentTraits<T>::GetSignature();
    EntityManager::Get().SetSignature(_id, signature);

    // remove from special deleter list
    _deleteComponent.erase(std::type_index(typeid(T)));
  }
}
