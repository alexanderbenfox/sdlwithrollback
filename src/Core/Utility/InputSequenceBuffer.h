#pragma once
#include "Core/Utility/TrieNode.h"
#include "Core/InputState.h"

struct BufferItem
{
  std::list<InputState> sequence;

  friend bool operator==(const BufferItem& k1, const BufferItem& k2)
  {
    if (k1.sequence.size() != k2.sequence.size())
      return false;
    return std::equal(k1.sequence.begin(), k1.sequence.end(), k2.sequence.begin(), [](InputState a, InputState b) { return a == b; });
  }

  // hasher
  std::size_t operator()(BufferItem item) const;
};

class InputSequenceBuffer
{
public:
  InputSequenceBuffer(int limit, const TrieNode<InputState, SpecialInputState>& dict) : _limit(limit), _dictionary(dict) {}

  void PushInput(const InputState& input);

  SpecialInputState const& GetLastSpecialInput() const;

  void Clear();


private:
  TrieNode<InputState, SpecialInputState> _dictionary;
  std::list<InputState> _latestCompletedSequence = {};
  //std::vector<std::pair<int, std::list<InputState>>> _prefixes;
  std::unordered_map<BufferItem, int, BufferItem> _prefixes;

  SpecialInputState _latestInput = SpecialInputState::NONE;

  int _limit;
};
