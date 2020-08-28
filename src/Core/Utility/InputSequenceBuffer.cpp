#include "Core/Utility/InputSequenceBuffer.h"
#include "Core/Utility/TypeTraits.h"

#include <algorithm>

//______________________________________________________________________________
void Serializer<std::list<InputState>>::Serialize(std::ostream& os, const std::list<InputState>& item)
{
  int inputSize = static_cast<int>(item.size());
  Serializer<int>::Serialize(os, inputSize);
  for (InputState input : item)
  {
    Serializer<InputState>::Serialize(os, input);
  }
}

//______________________________________________________________________________
void Serializer<std::list<InputState>>::Deserialize(std::istream& is, std::list<InputState>& item)
{
  int inputSize;
  Serializer<int>::Deserialize(is, inputSize);

  item.clear();
  for (int i = 0; i < inputSize; i++)
  {
    InputState input;
    Serializer<InputState>::Deserialize(is, input);
    item.push_back(input);
  }
}

//______________________________________________________________________________
std::size_t BufferItem::operator()(BufferItem item) const
{
  std::size_t hash = 0;
  for (auto& state : sequence)
    hash_combine(hash, static_cast<std::size_t>(state));
  return hash;
}

//______________________________________________________________________________
bool operator<(const BufferItem& lhs, const BufferItem& rhs)
{
  if (lhs.sequence.size() < rhs.sequence.size())
    return true;
  if (lhs.sequence.size() > rhs.sequence.size())
    return false;

  auto it2 = rhs.sequence.begin();
  for (auto it1 = lhs.sequence.begin(); it1 != lhs.sequence.end() && it2 != rhs.sequence.end(); ++it1, ++it2)
  {
    if (*it1 != *it2)
    {
      return (int)*it1 < (int)*it2;
    }
  }
  return true;
}

//______________________________________________________________________________
void Serializer<std::unordered_map<BufferItem, int, BufferItem>>::Serialize(std::ostream& os, const std::unordered_map<BufferItem, int, BufferItem>& item)
{
  // serialize the prefix map
  int prefixMapSize = static_cast<int>(item.size());
  Serializer<int>::Serialize(os, prefixMapSize);

  // this always needs to be serialized in the same order
  std::vector<BufferItem> orderedSet;
  for (const auto& prefix : item)
  {
    orderedSet.push_back(prefix.first);
  }

  std::sort(orderedSet.begin(), orderedSet.end());

  for (const BufferItem& key : orderedSet)
  {
    // serialize the key
    Serializer<std::list<InputState>>::Serialize(os, key.sequence);
    // serialize the lifetime
    Serializer<int>::Serialize(os, item.at(key));
  }
}

//______________________________________________________________________________
void Serializer<std::unordered_map<BufferItem, int, BufferItem>>::Deserialize(std::istream& is, std::unordered_map<BufferItem, int, BufferItem>& item)
{
  // deserialize the prefix map
  int prefixMapSize = 0;
  Serializer<int>::Deserialize(is, prefixMapSize);
  item.clear();
  for (int i = 0; i < prefixMapSize; i++)
  {
    // deserialize the key
    BufferItem key;
    Serializer<std::list<InputState>>::Deserialize(is, key.sequence);

    // deserialize the lifetime
    int lifetime;
    Serializer<int>::Deserialize(is, lifetime);

    // add prefix to map
    item[key] = lifetime;
  }
}

//______________________________________________________________________________
void InputSequenceBuffer::PushInput(const InputState& input)
{
  _latestCompletedSequence = {};

  // only concerned with directional input here, so we just look at the bottom 4 bits
  unsigned char chopped = (unsigned char)input << 4;
  InputState curr = (InputState)(chopped >> 4);

  std::vector<std::pair<BufferItem, int>> toAdd;
  for (auto it = _prefixes.begin(); it != _prefixes.end();)
  {
    it->second++;
    if (it->second > _limit)
    {
      it = _prefixes.erase(it);
      continue;
    }

    const std::list<InputState>& frontier = it->first.sequence;

    // copy frontier and push our next state onto the end
    std::list<InputState> searchInput = frontier;
    searchInput.push_back(curr);
    TrieReturnValue result = _dictionary.Search(searchInput);

    if (result == TrieReturnValue::Leaf)
    {
      _latestCompletedSequence = searchInput;
    }
    else if (result == TrieReturnValue::Branch)
    {
      toAdd.push_back(std::make_pair(BufferItem{ searchInput }, it->second));
    }
    it++;
  }

  // either insert new sequence or update existing age
  _lastPrefixes = _prefixes;
  for (auto& item : toAdd)
  {
    _prefixes[item.first] = item.second;
  }

  std::list<InputState> list = { curr };
  TrieReturnValue result = _dictionary.Search(list);
  if (result == TrieReturnValue::Branch)
  {
    _prefixes[BufferItem{ list }] = 0;
  }

  if (_latestCompletedSequence.empty())
    _latestInput = SpecialInputState::NONE;
  else
    _latestInput = _dictionary.GetKeyValue(_latestCompletedSequence);
}

//______________________________________________________________________________
void InputSequenceBuffer::RollbackLastInput()
{
  _prefixes = _lastPrefixes;
}

//______________________________________________________________________________
const SpecialInputState& InputSequenceBuffer::GetLastSpecialInput() const
{
  return _latestInput;
}

//______________________________________________________________________________
void InputSequenceBuffer::Clear()
{
  _latestCompletedSequence = {};
  _prefixes.clear();
  _latestInput = SpecialInputState::NONE;
}

//______________________________________________________________________________
void InputSequenceBuffer::Serialize(std::ostream& os) const
{
  // serialize the last input state
  Serializer<SpecialInputState>::Serialize(os, _latestInput);

  // serialize the last input sequence
  Serializer<std::list<InputState>>::Serialize(os, _latestCompletedSequence);

  Serializer<std::unordered_map<BufferItem, int, BufferItem>>::Serialize(os, _prefixes);
  Serializer<std::unordered_map<BufferItem, int, BufferItem>>::Serialize(os, _lastPrefixes);
}
//______________________________________________________________________________
void InputSequenceBuffer::Deserialize(std::istream& is)
{
  Serializer<SpecialInputState>::Deserialize(is, _latestInput);
  Serializer<std::list<InputState>>::Deserialize(is, _latestCompletedSequence);
  Serializer<std::unordered_map<BufferItem, int, BufferItem>>::Deserialize(is, _prefixes);
  Serializer<std::unordered_map<BufferItem, int, BufferItem>>::Deserialize(is, _lastPrefixes);
}
