#include "Core/InputState.h"

//______________________________________________________________________________
void operator|=(InputState& the, InputState other)
{
  the = (InputState)((unsigned char)the | (unsigned char)other);
}

//______________________________________________________________________________
void operator&=(InputState& the, InputState other)
{
  the = (InputState)((unsigned char)the & (unsigned char)other);
}

//______________________________________________________________________________
InputState operator&(InputState a, InputState b)
{
  return (InputState)((unsigned char)a & (unsigned char)b);
}

//______________________________________________________________________________
InputState operator|(InputState a, InputState b)
{
  return (InputState)((unsigned char)a | (unsigned char)b);
}

//______________________________________________________________________________
InputState operator~(InputState const& other)
{
  return (InputState)~((unsigned char)other);
}
