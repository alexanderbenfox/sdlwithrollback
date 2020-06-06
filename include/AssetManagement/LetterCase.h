#pragma once
#include "Resource.h"
#include <vector>

class GLTexture;

struct GLDrawOperation
{
  float x, y, lh;
  GLTexture* texture;
};

struct GLRenderString
{
  float x = 0;
  float y = 0;
  float ratio;
  std::vector<GLDrawOperation> textures;

};

//! Size of alphabet for our font
const size_t alphabetSize = 96;

struct LetterCase
{
  LetterCase();
  LetterCase(TTF_Font* font, size_t size);
  ~LetterCase();

  std::vector<GLDrawOperation> CreateStringField(const char* text, int fieldWidth, float lineHeight = 1.0f, float kerning = 1.0f);

  GLTexture* glyphs[alphabetSize];
  size_t _fontSize;

};
