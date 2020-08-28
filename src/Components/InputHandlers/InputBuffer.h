#pragma once
#include <functional>
#include <unordered_map>
#include "Core/Utility/InputSequenceBuffer.h"

#include "Core/Interfaces/Serializable.h"

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
class InputBuffer : ISerializable
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
  //! Swaps value with last value. If a different value is swapped in, sp buffer needs to be reevaluated
  void Swap(InputState input);
  //! evaluate possible special motions
  //SpecialInputState Evaluate(const TrieNode<InputState, SpecialInputState>& spMoveDict) const;
  SpecialInputState const& GetLastSpecialInput() const { return _spMovesBuffer.GetLastSpecialInput(); }
  //!
  void Clear();

  //! Override ISerializable functions
  void Serialize(std::ostream& os) const override;
  void Deserialize(std::istream& is) override;
  std::string Log() override;

private:
  std::vector<InputState> _buffer;
  InputSequenceBuffer _spMovesBuffer;
  int _limit;

};