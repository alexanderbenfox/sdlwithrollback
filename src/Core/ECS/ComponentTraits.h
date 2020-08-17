#pragma once
#include "Core/ECS/IComponent.h"

template <typename T>
class ComponentTraits
{
public:
  static const std::bitset<MAX_COMPONENTS>& GetSignature() { return signature; }

  //static ComponentEntityFnSet SerializationFns;


private:
  static int ID;
  static bool ID_Initialized;
  static std::bitset<MAX_COMPONENTS> signature;

  //! returns a deleter function when being called from entity wrapper
  static const std::function<void()> AddSelf(EntityID entity)
  {
    ComponentArray<T>::Get().Insert(entity, T());

    auto signature = EntityManager::Get().GetSignature(entity);
    signature |= GetSignature();
    EntityManager::Get().SetSignature(entity, signature);

    return [entity]() { ComponentTraits<T>::RemoveSelf(entity); };
  }

  static const void RemoveSelf(EntityID entity)
  {
    ComponentArray<T>::Get().Remove(entity);

    auto signature = EntityManager::Get().GetSignature(entity);
    signature &= ~GetSignature();
    EntityManager::Get().SetSignature(entity, signature);
  }

  static const void CopyDataFromEntity(EntityID entity, SBuffer& buffer)
  {
    if constexpr (std::is_base_of_v<ISerializable, T>)
      buffer = CopyDataToBuffer(ComponentArray<T>::Get().GetComponent(entity));
  }

  static const void CopyDataToEntity(EntityID entity, const SBuffer& buffer)
  {
    if constexpr (std::is_base_of_v<ISerializable, T>)
      CopyDataIntoComponent(ComponentArray<T>::Get().GetComponent(entity), buffer);
  }

  static const SBuffer& CopyDataToBuffer(const T& item)
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
  }
};

/*template <class T>
ComponentEntityFnSet ComponentTraits<T>::SerializationFns =
{
  std::function<std::function<void()>(EntityID)>([](EntityID e) { return AddSelf(e); }),
  std::function<void(EntityID)>([](EntityID e) { RemoveSelf(e); }),
  std::function<SBuffer(EntityID)>([](EntityID e) { SBuffer buffer; CopyDataFromEntity(e, buffer); return buffer; }),
  std::function<void(EntityID, const SBuffer&)>([](EntityID e, const SBuffer& b) { CopyDataToEntity(e, b); }),
  std::type_index(typeid(T))
};*/

template <class T>
int ComponentTraits<T>::ID = 0;

template <class T>
bool ComponentTraits<T>::ID_Initialized = false;

template <class T>
std::bitset<MAX_COMPONENTS> ComponentTraits<T>::signature =
ComponentIDGenerator::GenerateBitFlag(ComponentTraits<T>::ID_Initialized, ComponentTraits<T>::ID, ComponentTraits<T>::signature,
  {
  std::function<std::function<void()>(EntityID)>([](EntityID e) { return AddSelf(e); }),
  std::function<void(EntityID)>([](EntityID e) { RemoveSelf(e); }),
  std::function<SBuffer(EntityID)>([](EntityID e) { SBuffer buffer; CopyDataFromEntity(e, buffer); return buffer; }),
  std::function<void(EntityID, const SBuffer&)>([](EntityID e, const SBuffer& b) { CopyDataToEntity(e, b); }),
  std::type_index(typeid(T))
  }
);
