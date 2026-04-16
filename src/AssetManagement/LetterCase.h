#pragma once
#include "Resource.h"
#include "BlitOperation.h"
#include "Rendering/GLTexture.h"
#include "Core/Utility/TypeTraits.h"
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

struct FontKey
{
  size_t size;
  std::string path;

  friend bool operator==(const FontKey& k1, const FontKey& k2)
  {
    return k1.size == k2.size && k1.path == k2.path;
  }
};

template <> class std::hash<FontKey>
{
public:
  size_t operator()(const FontKey& key) const
  {
    std::size_t hashResult = 0;
    hash_combine(hashResult, key.size);
    hash_combine(hashResult, key.path);
    return hashResult;
  }
};
