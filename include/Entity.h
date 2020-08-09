#pragma once
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <functional>

#include "Core/Math/Vector2.h"
#include "Components/ComponentManager.h"
#include "Utils.h"

//! Global entity id counter
static int EntityID = 0;

//______________________________________________________________________________
//! Entity is the root of the component tree containing all of the positional information for the game object
class Entity : public std::enable_shared_from_this<Entity>
{
public:
  //! Increment creation id counter
  Entity() : ComponentBitFlag(0x0), _creationId(EntityID++) {}
  //! Default deleter
  ~Entity() = default;

  //! Removes all added components by calling list of deleter functions
  void RemoveAllComponents();
  //! Requests the game manager destroys this entity at the end of the frame
  void DestroySelf();
  //! Sets scale among all scalable components attached to this... THERE MUST BE A BETTER WAY
  void SetScale(Vector2<float> scale);
  //! Gets a unique id for the entity
  int GetID() const { return _creationId; }

  //! Retrieves the components of type specified or nullptr if there is no component of that type present
  template <typename T = IComponent> 
  std::shared_ptr<T> GetComponent();
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
  //! Helper for the systems
  template <typename ... T>
  std::tuple<std::add_pointer_t<T>...> MakeComponentTuple();

  //! Bit flag for the components currently attached
  uint64_t ComponentBitFlag;

protected:
  //! Creates a pointer to give to system tuple in MakeComponentTuple
  template <typename T>
  void SetPointerElement(T*& element) { element = GetComponent<T>().get(); }
  
  //! Don't remove deletion function here. This is used for removing components internally if we do not know the type
  template <typename T>
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

  //! Pointers to all components attached to the object. The component objects exist in their respective manager singleton objects
  std::unordered_map<std::type_index, std::shared_ptr<IComponent>> _components;
  //! Functors for deleting components en masse (without knowing type at runtime) uses RemoveComponentInternal
  std::unordered_map<std::type_index, std::function<void()>> _deleteComponent;
  //! This entity ID (and order of creation)
  int _creationId;

};

//______________________________________________________________________________
template <typename T>
inline std::shared_ptr<T> Entity::GetComponent()
{
  if (_components.find(std::type_index(typeid(T))) != _components.end())
    return std::static_pointer_cast<T>(_components[std::type_index(typeid(T))]);
  else return std::shared_ptr<T>(nullptr);
}

//______________________________________________________________________________
template <typename T>
inline void Entity::AddComponent()
{
  if (_components.find(std::type_index(typeid(T))) == _components.end())
  {
    ComponentBitFlag |= ComponentTraits<T>::GetSignature();
    _components.insert(std::make_pair(std::type_index(typeid(T)), ComponentManager<T>::Get().Create(std::shared_ptr<Entity>(shared_from_this()))));
    _deleteComponent.insert(std::make_pair(std::type_index(typeid(T)), [this]() { RemoveComponentInternal<T>(); }));

    // see if this needs to be added to the system
    CheckAgainstSystems(this);
  }
}

//______________________________________________________________________________
template <typename T>
inline void Entity::AddComponent(const ComponentInitParams<T>& initParams)
{
  AddComponent<T>();
  if (_components.find(std::type_index(typeid(T))) != _components.end())
  {
    ComponentInitParams<T>::Init(*std::dynamic_pointer_cast<T, IComponent>(_components[std::type_index(typeid(T))]), initParams);
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
  if (_components.find(std::type_index(typeid(T))) != _components.end())
  {
    ComponentBitFlag &= ~ComponentTraits<T>::GetSignature();
    ComponentManager<T>::Get().Erase(GetComponent<T>());
    _components.erase(std::type_index(typeid(T)));
    _deleteComponent.erase(std::type_index(typeid(T)));

    // see if this needs to be added to the system
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
template <typename ... T>
inline std::tuple<std::add_pointer_t<T>...> Entity::MakeComponentTuple()
{
  std::tuple<std::add_pointer_t<T>...> tuple;
  (SetPointerElement<T>(std::get<T*>(tuple)), ...);
  return tuple;
}

//______________________________________________________________________________
template <typename T>
inline void Entity::RemoveComponentInternal()
{
  if (_components.find(std::type_index(typeid(T))) != _components.end())
  {
    ComponentBitFlag &= ~ComponentTraits<T>::GetSignature();
    ComponentManager<T>::Get().Erase(GetComponent<T>());
    _components.erase(std::type_index(typeid(T)));
  }
}

//______________________________________________________________________________
template <typename T>
inline void Entity::AddComponentNoSystemCheck()
{
  if (_components.find(std::type_index(typeid(T))) == _components.end())
  {
    ComponentBitFlag |= ComponentTraits<T>::GetSignature();
    _components.insert(std::make_pair(std::type_index(typeid(T)), ComponentManager<T>::Get().Create(std::shared_ptr<Entity>(shared_from_this()))));
    _deleteComponent.insert(std::make_pair(std::type_index(typeid(T)), [this]() { RemoveComponentInternal<T>(); }));
  }
}

//______________________________________________________________________________
template <typename T>
inline void Entity::RemoveComponentNoSystemCheck()
{
  if (_components.find(std::type_index(typeid(T))) != _components.end())
  {
    ComponentBitFlag &= ~ComponentTraits<T>::GetSignature();
    ComponentManager<T>::Get().Erase(GetComponent<T>());
    _components.erase(std::type_index(typeid(T)));
    _deleteComponent.erase(std::type_index(typeid(T)));
  }
}
