#include "scene/faceselection.h"
#include "path/face.h"

namespace omm
{

FaceSelection::FaceSelection(Scene& scene)
    : m_scene(scene)
{

}

::transparent_set<Face> FaceSelection::faces() const
{
  return {};
}

}  // namespace omm
