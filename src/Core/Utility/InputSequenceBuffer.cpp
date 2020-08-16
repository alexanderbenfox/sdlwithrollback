#include "Core/Utility/InputSequenceBuffer.h"
#include "Core/Utility/TypeTraits.h"

//______________________________________________________________________________
std::size_t BufferItem::operator()(BufferItem item) const
{
  std::size_t hash = 0;
  for (auto& state : sequence)
    hash_combine(hash, static_cast<std::size_t>(state));
  return hash;
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
  for (auto& item : toAdd)
    _prefixes[item.first] = item.second;

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
