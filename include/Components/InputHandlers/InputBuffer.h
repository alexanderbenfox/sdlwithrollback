#pragma once
#include <functional>
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
  //! evaluate possible special motions
  SpecialInputState Evaluate(const TrieNode<InputState, SpecialInputState>& spMoveDict) const;
  //!
  void Clear();

private:
  std::vector<InputState> _buffer;
  int _limit;

};

// simple move dict to test this out
const TrieNode<InputState, SpecialInputState> UnivSpecMoveDict
{
  std::make_pair(std::list<InputState>{InputState::DOWN, InputState::DOWN | InputState::RIGHT, InputState::RIGHT}, SpecialInputState::QCF),
  std::make_pair(std::list<InputState>{InputState::DOWN, InputState::DOWN | InputState::LEFT, InputState::LEFT}, SpecialInputState::QCB),
  std::make_pair(std::list<InputState>{InputState::RIGHT, InputState::DOWN, InputState::RIGHT | InputState::DOWN}, SpecialInputState::DPF),
  std::make_pair(std::list<InputState>{InputState::LEFT, InputState::DOWN, InputState::LEFT | InputState::DOWN}, SpecialInputState::DPB),
  std::make_pair(std::list<InputState>{InputState::RIGHT, InputState::NONE, InputState::RIGHT}, SpecialInputState::RDash),
  std::make_pair(std::list<InputState>{InputState::LEFT, InputState::NONE, InputState::LEFT}, SpecialInputState::LDash)
};
