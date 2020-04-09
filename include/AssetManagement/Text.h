#pragma once
#include "Resource.h"

class TextResource : public Resource<SDL_Texture>
{
public:
  TextResource(Resource<TTF_Font> font, const char* text, SDL_Color color);
  virtual void Load() override;
  
};