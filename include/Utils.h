#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <iterator>

namespace StringUtils
{
  template <typename Out>
  static void Split(const std::string &s, char delim, Out result)
  {
    std::istringstream iss(s);
    std::string item;
    while (std::getline(iss, item, delim)) {
      *result++ = item;
    }
  }

  static std::vector<std::string> Split(const std::string &s, char delim)
  {
    std::vector<std::string> elems;
    Split(s, delim, std::back_inserter(elems));
    return elems;
  }

  static std::string Connect(std::vector<std::string>::iterator const& beg, std::vector<std::string>::iterator const& end, char delim)
  {
    std::stringstream ss("");
    auto emit = [&ss, delim, need_delim = false](std::string const& x) mutable
    {
      if (need_delim) ss << delim;
      ss << x;
      need_delim = true;
    };

    for (auto it = beg; it != end; ++it)
      emit(*it);
    return ss.str();
  }
}