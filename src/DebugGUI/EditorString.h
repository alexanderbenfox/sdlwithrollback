#pragma once
#include <string>

struct EditorString
{
public:
#ifdef _WIN32
  #define CPY(a, b) strcpy_s<bSize>(a, b);
#else
  #define CPY(a, b) strcpy(a, b);
#endif

  EditorString() = default;
  EditorString(const std::string& str);
  EditorString(std::string&& str);
  EditorString(const char* str);

  //! Overload typecast operator
  operator std::string() const { return _string; }
  std::string const& str() { return _string; }

  void DisplayEditable(const char* label);

private:
  //! String data
  std::string _string;

  //! Member variables for displaying text in editor
  static constexpr size_t bSize = 256;
  char _textBuffer[bSize] = {};
};
