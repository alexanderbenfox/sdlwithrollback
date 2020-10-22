#include "AnimationEditor.h"

void ActionEditor::EditHitboxExtentsInDisplay(Vector2<double> displaySize, Vector2<double> srcSize, Vector2<double> clickPos, Rect<double>& hitbox)
{
  // if no hitbox exists and there was a click, create a small hb at the click pos
  if (hitbox.Area() == 0)
  {
    const double newHitboxSideLength = 5.0;

    hitbox.beg = Vector2<double>((clickPos.x - newHitboxSideLength / 2.0) * srcSize.x / displaySize.x, (clickPos.y - newHitboxSideLength / 2.0) * srcSize.y / displaySize.y);
    hitbox.end = Vector2<double>((clickPos.x + newHitboxSideLength / 2.0) * srcSize.x / displaySize.x, (clickPos.y + newHitboxSideLength / 2.0) * srcSize.y / displaySize.y);
  }
  else
  {
    Vector2<double> drawBeg(hitbox.beg.x / srcSize.x * displaySize.x, hitbox.beg.y / srcSize.y * displaySize.y);
    Vector2<double> drawEnd(hitbox.end.x / srcSize.x * displaySize.x, hitbox.end.y / srcSize.y * displaySize.y);

    double distA = (drawBeg - clickPos).Magnitude();
    double distB = (drawEnd - clickPos).Magnitude();

    // modify the lesser one
    if (distA <= distB)
    {
      drawBeg = clickPos;
      hitbox.beg = Vector2<double>(drawBeg.x * srcSize.x / displaySize.x, drawBeg.y * srcSize.y / displaySize.y);
    }
    else
    {
      drawEnd = clickPos;
      hitbox.end = Vector2<double>(drawEnd.x * srcSize.x / displaySize.x, drawEnd.y * srcSize.y / displaySize.y);
    }
  }
}