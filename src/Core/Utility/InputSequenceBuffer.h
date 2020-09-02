#pragma once
#include "Core/Utility/TrieNode.h"
#include "Core/InputState.h"
#include "Core/Interfaces/Serializable.h"

template <> struct Serializer<std::list<InputState>>
{
  static void Serialize(std::ostream& os, const std::list<InputState>& item);
  static void Deserialize(std::istream& is, std::list<InputState>& item);
};

struct BufferItem
{
  //!
  std::list<InputState> sequence;
  //!
  friend bool operator==(const BufferItem& k1, const BufferItem& k2)
  {
    if (k1.sequence.size() != k2.sequence.size())
      return false;
    return std::equal(k1.sequence.begin(), k1.sequence.end(), k2.sequence.begin(), [](InputState a, InputState b) { return a == b; });

    /*auto it2 = k2.sequence.begin();
    for (auto it1 = k1.sequence.begin(); it1 != k1.sequence.end() && it2 != k2.sequence.end(); ++it1, ++it2)
    {
      if (*it1 != *it2)
        return false;
    }
    return true;*/
  }
  //! hasher
  std::size_t operator()(BufferItem item) const;
  //!
  operator std::string() const
  {
    std::string s = "{";
    for (const auto& item : sequence)
      s += (" " + std::to_string((int)item));
    s += " }";
    return s;
  }

};

bool operator<(const BufferItem& lhs, const BufferItem& rhs);

template <> struct Serializer<std::unordered_map<BufferItem, int, BufferItem>>
{
  static void Serialize(std::ostream& os, const std::unordered_map<BufferItem, int, BufferItem>& item);
  static void Deserialize(std::istream& is, std::unordered_map<BufferItem, int, BufferItem>& item);
};

class InputSequenceBuffer : ISerializable
{
public:
  InputSequenceBuffer(int limit, const TrieNode<InputState, SpecialInputState>& dict) : _limit(limit), _dictionary(dict) {}

  void PushInput(const InputState& input);

  void RollbackLastInput();

  SpecialInputState const& GetLastSpecialInput() const;

  void Clear();

  void Serialize(std::ostream& os) const override;
  void Deserialize(std::istream& is) override;
  std::string Log() override
  {
    std::stringstream ss;
    ss << "InputSequenceBuffer\n";

    std::string s = "{";
    for (const auto& item : _latestCompletedSequence)
      s += (" " + std::to_string((int)item));
    s += " }";

    ss << "\tLast completed sequence: " << s;
    ss << "\tCurrent prefixes: ";
    for (auto pair : _prefixes)
    {
      std::string strang = pair.first;
      strang += (" : " + std::to_string(pair.second));
      ss << strang;
    }
    ss << "\n\tLast prefixes: ";
    for (auto pair : _lastPrefixes)
    {
      std::string strang = pair.first;
      strang += (" : " + std::to_string(pair.second));
      ss << strang;
    }
    ss << "\n";
    return ss.str();
  }

private:
  TrieNode<InputState, SpecialInputState> _dictionary;
  //std::vector<std::pair<int, std::list<InputState>>> _prefixes;
  std::unordered_map<BufferItem, int, BufferItem> _prefixes;

  //! copy of prefix map for roll back events
  std::unordered_map<BufferItem, int, BufferItem> _lastPrefixes;

  SpecialInputState _latestInput = SpecialInputState::NONE;
  std::list<InputState> _latestCompletedSequence = {};

  int _limit;
};
