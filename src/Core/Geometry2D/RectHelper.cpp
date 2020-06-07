#include "Core/Geometry2D/RectHelper.h"

//______________________________________________________________________________
void operator|=(CollisionSide& a, CollisionSide b)
{
  a = (CollisionSide)((unsigned char)a | (unsigned char)b);
}

//______________________________________________________________________________
CollisionSide operator|(CollisionSide a, CollisionSide b)
{
  return (CollisionSide)((unsigned char)a | (unsigned char)b);
}

//______________________________________________________________________________
void operator&=(CollisionSide& a, CollisionSide b)
{
  a = (CollisionSide)((unsigned char)a & (unsigned char)b);
}

//______________________________________________________________________________
CollisionSide operator&(CollisionSide a, CollisionSide b)
{
  return (CollisionSide)((unsigned char)a & (unsigned char)b);
}

//______________________________________________________________________________
CollisionSide operator~(CollisionSide a)
{
  return (CollisionSide)~((unsigned char)a);
}
