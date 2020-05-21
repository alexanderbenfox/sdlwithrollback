#pragma once
#include "Resource.h"
#include "Rendering/GLTexture.h"
#include <vector>

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
  LetterCase() : _fontSize(0)
  {
    for (int i = 0; i < alphabetSize; ++i)
    {
      glyphs[i] = nullptr;
    }
  }

  LetterCase(TTF_Font* font, size_t size) : _fontSize(size)
  {
    SDL_Surface* surf;

    // This could easily be modified for utf-8 by using utf-8 char codes 
    // and TTF_RenderGlyph_Blended() instead. 
    // Just get the ASCII latin1 charset
    char s[] = " ";
    for (char i = 0; i < alphabetSize; ++i)
    {
      s[0] = i + ' ';
      surf = TTF_RenderText_Blended(font, s, SDL_Color{ 255, 255, 255 });
      surf->refcount++; // SDL2: prevent segfault on free
      glyphs[i] = new GLTexture(surf);
      SDL_FreeSurface(surf);
    }
  }

  std::vector<GLDrawOperation> CreateStringField(const char* text, int fieldWidth, float lineHeight = 1.0f, float kerning = 1.0f)
  {
    GLRenderString string;
    string.ratio = _fontSize / glyphs[0]->height;

    char c;
    for (int i = 0; text[i] != '\0'; i++)
    {
      // deal with linebreak
      if (text[i] == '\n')
      {
        string.x = 0;
        string.y += _fontSize * lineHeight;
      }

      // clamp to ' ' (96)
      c = text[i] < ' ' ? ' ' : text[i];

      // line wrap
      if (c != ' ' && abs((int)string.x) >= fieldWidth)
      {
        string.x = 0;
        string.y += _fontSize * lineHeight;
      }

      // shift to match our letter case indices
      c -= ' ';

      string.textures.push_back({ string.x, string.y, static_cast<float>(_fontSize), glyphs[c] });
      
      string.x += glyphs[c]->width * (_fontSize / glyphs[c]->height) * kerning;
    }
    return string.textures;
  }

  ~LetterCase()
  {
    if (glyphs[0] != nullptr)
    {
      for (int i = 0; i < alphabetSize; ++i)
      {
        delete glyphs[i];
      }
    }
  }
  GLTexture* glyphs[alphabetSize];
  size_t _fontSize;

};
