#pragma once
#include "Resource.h"
#include "LetterCase.h"
#include "Core/Utility/TypeTraits.h"

class TextResource : public Resource<SDL_Texture>
{
public:
  TextResource() = default;
  TextResource(TTF_Font* font, const char* text, SDL_Color color);
  virtual ~TextResource() {}
  virtual void Load() override;
  
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

const size_t FontSizes[8] = { 12, 16, 20, 24, 32, 40, 48, 72 };
