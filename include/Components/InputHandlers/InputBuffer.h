#pragma once
#include <functional>
#include <unordered_map>
#include "Utils.h"

//______________________________________________________________________________
enum class InputState : unsigned char
{
  NONE = 0x00,
  UP = 0x01,
  DOWN = 0x02,
  RIGHT = 0x04,
  LEFT = 0x08,
  BTN1 = 0x10,
  BTN2 = 0x20,
  BTN3 = 0x40,
  BTN4 = 0x80
};

template <> struct std::hash<InputState>
{
  std::size_t operator()(const InputState& k) const
  {
    return hash<unsigned char>()((unsigned char)k);
  }
};

//______________________________________________________________________________
enum class SpecialInputState : unsigned char
{
  NONE = 0x00,
  QCF = 0x01, // quarter circle forward
  QCB = 0x02, // quarter circle back
  DPF = 0x04, // dragon punch forward
  DPB = 0x08, // dragon punch back
  RDash = 0x10, // right dash input
  LDash = 0x20 // left dash input
};

//______________________________________________________________________________
void operator|=(InputState& the, InputState other);
//______________________________________________________________________________
void operator&=(InputState& the, InputState other);
//______________________________________________________________________________
InputState operator&(InputState a, InputState b);
//______________________________________________________________________________
InputState operator|(InputState a, InputState b);
//______________________________________________________________________________
InputState operator~(InputState const& other);

//______________________________________________________________________________
static bool HasState(const InputState& state, InputState other) { return (state & other) == other; }


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
  std::size_t operator()(BufferItem item) const
  {
    std::size_t hash = 0;
    for (auto& state : sequence)
      hash_combine(hash, static_cast<std::size_t>(state));
    return hash;
  }
};

class SpecialMovesBuffer
{
public:
  SpecialMovesBuffer(int limit, const TrieNode<InputState, SpecialInputState>& dict) : _limit(limit), _dictionary(dict) {}

  void PushInput(const InputState& input);

  SpecialInputState const& GetLastSpecialInput() const;

  void Clear();


private:
  const TrieNode<InputState, SpecialInputState>& _dictionary;
  std::list<InputState> _latestCompletedSequence = {};
  //std::vector<std::pair<int, std::list<InputState>>> _prefixes;
  std::unordered_map<BufferItem, int, BufferItem> _prefixes;

  SpecialInputState _latestInput = SpecialInputState::NONE;

  int _limit;
};



// simple move dict to test this out
const TrieNode<InputState, SpecialInputState> UnivSpecMoveDict
{
  std::make_pair(std::list<InputState>{InputState::DOWN, InputState::DOWN | InputState::RIGHT, InputState::RIGHT}, SpecialInputState::QCF),
  std::make_pair(std::list<InputState>{InputState::DOWN, InputState::DOWN | InputState::LEFT, InputState::LEFT}, SpecialInputState::QCB),
  std::make_pair(std::list<InputState>{InputState::RIGHT, InputState::DOWN, InputState::RIGHT | InputState::DOWN}, SpecialInputState::DPF),
  std::make_pair(std::list<InputState>{InputState::LEFT, InputState::DOWN, InputState::LEFT | InputState::DOWN}, SpecialInputState::DPB),
  std::make_pair(std::list<InputState>{InputState::NONE, InputState::RIGHT, InputState::NONE, InputState::RIGHT}, SpecialInputState::RDash),
  std::make_pair(std::list<InputState>{InputState::NONE, InputState::LEFT, InputState::NONE, InputState::LEFT}, SpecialInputState::LDash)
};

//! Input buffer class for storing 
class InputBuffer
{
public:
  //! construct with a limit to the size of the buffer
  InputBuffer(int limit);
  //! push new input state into the buffer and remove oldest state from buffer
  void Push(InputState item);
  //! gets the most recently added item
  InputState const& Latest() const { return _buffer.back(); }
  //! gets most recently added item if it was just pressed this frame
  InputState LatestPressed() const;
  //! evaluate possible special motions
  //SpecialInputState Evaluate(const TrieNode<InputState, SpecialInputState>& spMoveDict) const;
  SpecialInputState const& GetLastSpecialInput() const { return _spMovesBuffer.GetLastSpecialInput(); }
  //!
  void Clear();

private:
  std::vector<InputState> _buffer;
  SpecialMovesBuffer _spMovesBuffer;
  int _limit;

};