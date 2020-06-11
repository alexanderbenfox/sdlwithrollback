#pragma once
#include <vector>
#include <streambuf>

template<typename CharT, typename TraitsT = std::char_traits<CharT> >
class ByteBuffer : public std::basic_streambuf<CharT, TraitsT>
{
public:
  ByteBuffer(std::vector<CharT>& vec)
  {
    setg(vec.data(), vec.data(), vec.data() + vec.size());
  }
};

//______________________________________________________________________________
class IGameState
{
public:
  virtual ~IGameState() {}

  virtual void SaveGameState(ByteBuffer<char>& data) = 0;
  virtual void LoadGameState(ByteBuffer<char>& data) = 0;
};
