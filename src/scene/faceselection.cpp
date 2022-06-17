#include "scene/faceselection.h"
#include "path/face.h"

namespace omm
{

FaceSelection::FaceSelection(Scene&)
{

}

void FaceSelection::set_selected(const Face& face, bool is_selected)
{
  if (is_selected) {
    select(face);
  } else {
    deselect(face);
  }
}

void FaceSelection::select(const Face& face)
{
  m_selection.insert(face);
}

void FaceSelection::deselect(const Face& face)
{
  m_selection.erase(face);
}

bool FaceSelection::is_selected(const Face& face)
{
  return m_selection.contains(face);
}

void FaceSelection::clear()
{
  m_selection.clear();
}

}  // namespace omm
