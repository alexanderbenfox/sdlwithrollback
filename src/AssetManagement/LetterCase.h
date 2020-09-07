#pragma once
#include "Resource.h"
#include "BlitOperation.h"
#include "Rendering/GLTexture.h"
#include <vector>

struct GLDrawOperation
{
  float x, y, lh;
  Resource<GLTexture>* texture;
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

enum class TextAlignment
{
  Left,
  Right,
  Centered,
  Justified
};

struct LetterCase
{
  LetterCase();
  LetterCase(TTF_Font* font, size_t size);
  ~LetterCase();

  //! right now only deals with left aligned and centered
  std::vector<GLDrawOperation> CreateStringField(const char* text, int fieldWidth, TextAlignment alignment, float lineHeight = 1.0f, float kerning = 1.0f);

  Resource<GLTexture> glyphs[alphabetSize];
  size_t _fontSize;

};
