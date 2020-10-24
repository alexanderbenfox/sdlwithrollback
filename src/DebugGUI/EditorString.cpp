#include "EditorString.h"
#include "../imgui/imgui.h"

EditorString::EditorString(const std::string& str)
{
  _string = str;
  strcpy_s<bSize>(_textBuffer, str.c_str());
}

EditorString::EditorString(std::string&& str)
{
  _string = str;
  strcpy_s<bSize>(_textBuffer, str.c_str());
}

EditorString::EditorString(const char* str)
{
  _string = str;
  strcpy_s<bSize>(_textBuffer, str);
}

void EditorString::DisplayEditable(const char* label)
{
  if (ImGui::InputText(label, _textBuffer, bSize))
  {
    _string = std::string(_textBuffer);
  }
}
