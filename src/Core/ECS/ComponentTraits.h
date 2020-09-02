#pragma once
#include "Core/ECS/IComponent.h"
#include "Core/ECS/ECSCoordinator.h"

//! Mainly used to map component manipulation function to an ID value at runtime
template <typename T = IComponent>
class ComponentTraits
{
public:
  //! Static getter
  static ComponentTraits& Get()
  {
    static ComponentTraits t;
    return t;
  }
  //! Gets the signature for this component type
  const std::bitset<MAX_COMPONENTS>& GetSignature() { return _signature; }

private:
  //! Initializes component ID and signature using ECSCoordinator
  ComponentTraits();

  //! returns a deleter function when being called from entity wrapper
  const std::function<void()> AddSelf(EntityID entity);
  const void RemoveSelf(EntityID entity);
  const void CopyDataFromEntity(EntityID entity, SBuffer& buffer);
  const void CopyDataToEntity(EntityID entity, const SBuffer& buffer);
  const void Serialize(EntityID entity, std::ostream& os);
  const void Deserialize(EntityID entity, std::istream& is);
  const std::string LogSelf(EntityID entity);

  int _ID = 0;
  std::bitset<MAX_COMPONENTS> _signature;

  /*static const SBuffer& CopyDataToBuffer(const T& item)
  {
    std::iostream stream(std::iostream::binary);
    item.Serialize(stream);

    return SBuffer(std::istreambuf_iterator<char>(stream), {});
  }

  static const void CopyDataIntoComponent(T& item, const SBuffer& buffer)
  {
    std::iostream stream(std::iostream::binary);
    stream.write(buffer.data(), buffer.size());

    item.Deserialize(stream);
  }*/

};

template <typename T>
inline ComponentTraits<T>::ComponentTraits()
{
  _signature = ECSCoordinator::Get().RegisterComponent(_ID,
  {
    std::function<std::function<void()>(EntityID)>([this](EntityID e) { return AddSelf(e); }),
    std::function<void(EntityID)>([this](EntityID e) { RemoveSelf(e); }),
    std::function<void(EntityID, std::ostream&)>([this](EntityID e, std::ostream& os) { Serialize(e, os); }),
    std::function<void(EntityID, std::istream&)>([this](EntityID e, std::istream& is) { Deserialize(e, is); }),
    std::function<std::string(EntityID)>([this](EntityID e) { return LogSelf(e); }),
    //std::function<SBuffer(EntityID)>([](EntityID e) { SBuffer buffer; CopyDataFromEntity(e, buffer); return buffer; }),
    //std::function<void(EntityID, const SBuffer&)>([](EntityID e, const SBuffer& b) { CopyDataToEntity(e, b); }),
    std::type_index(typeid(T))
  });
}

template <typename T>
const std::function<void()> ComponentTraits<T>::AddSelf(EntityID entity)
{
  ComponentArray<T>::Get().Insert(entity, T());

  auto signature = EntityManager::Get().GetSignature(entity);
  signature |= GetSignature();
  EntityManager::Get().SetSignature(entity, signature);

  return [entity]() { ComponentTraits<T>::Get().RemoveSelf(entity); };
}

template <typename T>
const void ComponentTraits<T>::RemoveSelf(EntityID entity)
{
  ComponentArray<T>::Get().Remove(entity);

  auto signature = EntityManager::Get().GetSignature(entity);
  signature &= ~GetSignature();
  EntityManager::Get().SetSignature(entity, signature);
}

template <typename T>
const void ComponentTraits<T>::CopyDataFromEntity(EntityID entity, SBuffer& buffer)
{
  if constexpr (std::is_base_of_v<ISerializable, T>)
    buffer = CopyDataToBuffer(ComponentArray<T>::Get().GetComponent(entity));
}

template <typename T>
const void ComponentTraits<T>::CopyDataToEntity(EntityID entity, const SBuffer& buffer)
{
  if constexpr (std::is_base_of_v<ISerializable, T>)
    CopyDataIntoComponent(ComponentArray<T>::Get().GetComponent(entity), buffer);
}


template <typename T>
const void ComponentTraits<T>::Serialize(EntityID entity, std::ostream& os)
{
  if constexpr (std::is_base_of_v<ISerializable, T>)
    ComponentArray<T>::Get().GetComponent(entity).Serialize(os);
}

template <typename T>
const void ComponentTraits<T>::Deserialize(EntityID entity, std::istream& is)
{
  if constexpr (std::is_base_of_v<ISerializable, T>)
    ComponentArray<T>::Get().GetComponent(entity).Deserialize(is);
}

template <typename T>
const std::string ComponentTraits<T>::LogSelf(EntityID entity)
{
  if constexpr (std::is_base_of_v<ISerializable, T>)
    return ComponentArray<T>::Get().GetComponent(entity).Log();
  else return "";
}

