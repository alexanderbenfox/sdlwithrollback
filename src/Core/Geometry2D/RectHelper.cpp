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

//______________________________________________________________________________
std::ostream& operator<<(std::ostream& os, const CollisionSide& rb)
{
  os.write((const char*)&rb, sizeof(rb));
  return os;
}

//______________________________________________________________________________
std::istream& operator>>(std::istream& is, CollisionSide& rb)
{
  is.read((char*)&rb, sizeof(rb));
  return is;
}
