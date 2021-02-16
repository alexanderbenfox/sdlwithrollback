//! Defines InputState and SpecialInputState
#pragma once
#include <functional>

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

static const char* InputStateStrings[] = { "NONE", "UP", "DOWN", "RIGHT", "LEFT", "BTN1", "BTN2", "BTN3", "BTN4" };

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
template <> struct std::hash<InputState>
{
  std::size_t operator()(const InputState& k) const
  {
    return hash<unsigned char>()((unsigned char)k);
  }
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
