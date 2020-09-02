#pragma once
#include <initializer_list>
#include <list>
#include <unordered_map>

enum class TrieReturnValue
{
  Leaf, Branch, None
};

// real simple trie implementation for special moves
template <typename T, typename V>
class TrieNode
{
public:
  //! Constructor
  TrieNode() : _endpoint(false), _value() {}
  TrieNode(std::initializer_list<std::pair<std::list<T>, V>> initList);

  void InsertSequence(const std::list<T>& sequenceKey, V value);
  TrieReturnValue Search(const std::list<T>& sequence) const;
  V const& GetKeyValue(const std::list<T>& key) const;

protected:
  // mark as friend so root can manipulate the other nodes
  friend class TrieNode<T, V>;

  const TrieNode<T, V>* Lookup(const std::list<T>& sequence) const;

  std::unordered_map<T, TrieNode<T, V>> _states;
  bool _endpoint;
  V _value;

};

template <typename T, typename V>
inline TrieNode<T, V>::TrieNode(std::initializer_list<std::pair<std::list<T>, V>> initList) : _endpoint(false)
{
  for (auto& item : initList)
    InsertSequence(item.first, item.second);
}

template <typename T, typename V>
inline void TrieNode<T, V>::InsertSequence(const std::list<T>& sequenceKey, V value)
{
  // start from root node
  TrieNode<T, V>* curr = this;
  for (auto& item : sequenceKey)
  {
    // create a new node if path doesn't exists
    if (curr->_states.find(item) == curr->_states.end())
      curr->_states.insert(std::make_pair(item, TrieNode()));

    // go to next node
    curr = &(curr->_states[item]);
  }

  // mark current node as leaf
  curr->_endpoint = true;
  // set the value
  curr->_value = value;
}

template <typename T, typename V>
inline TrieReturnValue TrieNode<T, V>::Search(const std::list<T>& sequence) const
{
  const TrieNode* item = Lookup(sequence);
  if (item == nullptr)
    return TrieReturnValue::None;

  // if not endpoint, it could still be a completed sequence
  return item->_endpoint ? TrieReturnValue::Leaf : TrieReturnValue::Branch;
}

template <typename T, typename V>
inline const V& TrieNode<T, V>::GetKeyValue(const std::list<T>& key) const
{
  const TrieNode* item = Lookup(key);
  if (item)
    return item->_value;
  return _value;
}

template <typename T, typename V>
inline const TrieNode<T, V>* TrieNode<T, V>::Lookup(const std::list<T>& sequence) const
{
  const TrieNode* curr = this;
  for (const auto& item : sequence)
  {
    // if not found, return nullptr
    if (curr->_states.find(item) == curr->_states.end())
      return nullptr;
    // go to next node
    curr = &((*curr->_states.find(item)).second);
  }
  return curr;
}
