#pragma once
#include "../imgui/imgui.h"
#include "Core/Geometry2D/Rect.h"

template <typename T>
struct EditableRectTraits
{
  static void OnClick(Rect<T>& current, const Vector2<T>& position);
  static void Clear(Rect<T>& geometry);
  static bool HasData(const Rect<T>& geometry);
  static Rect<T> Scale(const Rect<T>& geometry, const Vector2<T>& srcCanvas, const Vector2<T>& dstCanvas);
  static void DrawGeometry(const Rect<T>& geometry, const Vector2<float>& windowPosition);
};

template <typename T>
struct EditablePointTraits
{
  static void OnClick(Vector2<T>& current, const Vector2<T>& position);
  static void Clear(Vector2<T>& geometry);
  static bool HasData(const Vector2<T>& geometry);
  static Vector2<T> Scale(const Vector2<T>& geometry, const Vector2<T>& srcCanvas, const Vector2<T>& dstCanvas);
  static void DrawGeometry(const Vector2<T>& geometry, const Vector2<float>& windowPosition);
};

template <template <typename> typename Geometry, typename T>
struct EditableTraits { typedef EditableRectTraits<T> F; };

template <typename T>
struct EditableTraits<Rect, T> { typedef EditableRectTraits<T> F; };

template <typename T>
struct EditableTraits<Vector2, T> { typedef EditablePointTraits<T> F; };

template <typename T, template <typename> typename Geometry>
class IEditorGeometryCanvas
{
public:
  IEditorGeometryCanvas() = default;
  IEditorGeometryCanvas(Vector2<T> canvas) : _canvasSize(canvas) {}

  void Import(const Geometry<T>& geom, const Vector2<T>& srcCanvasSize) { _geom = EditableTraits<Geometry, T>::F::Scale(geom, srcCanvasSize, _storedSize); }

  Geometry<T> Export(const Vector2<T>& dstCanvasSize) const { return EditableTraits<Geometry, T>::F::Scale(_geom, _storedSize, dstCanvasSize); }

  void ClearGeometry() { EditableTraits<Geometry, T>::F::Clear(_geom); }
  bool UserDataExists() const { return EditableTraits<Geometry, T>::F::HasData(_geom); }

  void DisplayAtPosition(Vector2<float> windowPosition);

  void SetCanvasSize(Vector2<T> canvas) { _canvasSize = canvas; }

protected:
  Geometry<T> _geom;
  Vector2<T> _canvasSize;
  Vector2<T> _storedSize = Vector2<T>(100.0, 100.0);

};

//______________________________________________________________________________
template <typename T>
inline void EditableRectTraits<T>::OnClick(Rect<T>& rect, const Vector2<T>& position)
{
  {
    // if no hitbox exists and there was a click, create a small hb at the click pos
    if (rect.Area() == 0)
    {
      const double newSideLength = 5.0;

      rect.beg = Vector2<double>(position.x - newSideLength / 2.0, position.y - newSideLength / 2.0);
      rect.end = Vector2<double>(position.x + newSideLength / 2.0, position.y + newSideLength / 2.0);
    }
    else
    {
      double distA = (rect.beg - position).Magnitude();
      double distB = (rect.end - position).Magnitude();

      // modify the lesser one
      if (distA <= distB)
      {
        rect.beg = position;
      }
      else
      {
        rect.end = position;
      }
    }
  }
}

//______________________________________________________________________________
template <typename T>
inline void EditableRectTraits<T>::Clear(Rect<T>& rect)
{
  rect = Rect<T>();
}


//______________________________________________________________________________
template <typename T>
inline bool EditableRectTraits<T>::HasData(const Rect<T>& rect)
{
  return rect.Area() != 0.0;
}

//______________________________________________________________________________
template <typename T>
inline Rect<T> EditableRectTraits<T>::Scale(const Rect<T>& rect, const Vector2<T>& srcCanvas, const Vector2<T>& dstCanvas)
{
  const Vector2<T> scaler = dstCanvas / srcCanvas;
  return Rect<T>(rect.beg * scaler, rect.end * scaler);
}

//______________________________________________________________________________
template <typename T>
inline void EditableRectTraits<T>::DrawGeometry(const Rect<T>& rect, const Vector2<float>& windowPosition)
{
  if (rect.Area() != 0)
  {
    ImDrawList* draws = ImGui::GetWindowDrawList();
    draws->AddRect(ImVec2(windowPosition.x + static_cast<float>(rect.beg.x), windowPosition.y + static_cast<float>(rect.beg.y)),
      ImVec2(windowPosition.x + static_cast<float>(rect.end.x), windowPosition.y + static_cast<float>(rect.end.y)),
      IM_COL32(255, 0, 0, 255));
  }
}

//______________________________________________________________________________
template <typename T>
inline void EditablePointTraits<T>::OnClick(Vector2<T>& pt, const Vector2<T>& position)
{
  pt = position;
}

//______________________________________________________________________________
template <typename T>
inline void EditablePointTraits<T>::Clear(Vector2<T>& pt)
{
  pt = Vector2<T>::Zero;
}

//______________________________________________________________________________
template <typename T>
inline bool EditablePointTraits<T>::HasData(const Vector2<T>& pt)
{
  return pt.x != 0 || pt.y != 0;
}

//______________________________________________________________________________
template <typename T>
inline Vector2<T> EditablePointTraits<T>::Scale(const Vector2<T>& pt, const Vector2<T>& srcCanvas, const Vector2<T>& dstCanvas)
{
  const Vector2<T> scaler = dstCanvas / srcCanvas;
  Vector2<T> scaled = pt;
  scaled.x *= scaler.x;
  scaled.y *= scaler.y;
  return scaled;
}

//______________________________________________________________________________
template <typename T>
inline void EditablePointTraits<T>::DrawGeometry(const Vector2<T>& pt, const Vector2<float>& windowPosition)
{
  const T radius = 3.0;

  if (HasData(pt))
  {
    ImDrawList* draws = ImGui::GetWindowDrawList();
    ImVec2 center{ windowPosition.x + static_cast<float>(pt.x), windowPosition.y + static_cast<float>(pt.y) };
    draws->AddCircleFilled(center, radius + 2.0, IM_COL32(255, 255, 255, 255));
    draws->AddCircleFilled(center, radius, IM_COL32(255, 0, 0, 255));
  }
}

//______________________________________________________________________________
template <typename T, template <typename> typename Geometry>
inline void IEditorGeometryCanvas<T, Geometry>::DisplayAtPosition(Vector2<float> windowPosition)
{
  if (ImGui::GetIO().MouseClicked[0])
  {
    ImVec2 cursor = ImGui::GetMousePos();
    Vector2<T> cursorPos(cursor.x - windowPosition.x, cursor.y - windowPosition.y);

    // check if its inside the canvas
    if (!(cursorPos.x < 0 || cursorPos.y < 0 || cursorPos.x > _canvasSize.x || cursorPos.y > _canvasSize.y))
    {
      cursorPos = EditablePointTraits<T>::Scale(cursorPos, _canvasSize, _storedSize);
      EditableTraits<Geometry, T>::F::OnClick(_geom, cursorPos);
    }
  }
  else if (ImGui::GetIO().MouseClicked[1])
  {
    EditableTraits<Geometry, T>::F::Clear(_geom);
  }

  // draw geom scaled to canvas
  Geometry<T> drawnGeom = EditableTraits<Geometry, T>::F::Scale(_geom, _storedSize, _canvasSize);
  EditableTraits<Geometry, T>::F::DrawGeometry(drawnGeom, windowPosition);

}
