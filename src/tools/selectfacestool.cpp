#include "tools/selectfacestool.h"
#include "scene/scene.h"
#include "path/face.h"
#include "scene/faceselection.h"

namespace omm
{

QString SelectFacesTool::type() const
{
  return TYPE;
}

SceneMode SelectFacesTool::scene_mode() const
{
  return SceneMode::Face;
}

Vec2f SelectFacesTool::selection_center() const
{
  return Vec2f{};
}

void SelectFacesTool::transform_objects(ObjectTransformation transformation)
{
  Q_UNUSED(transformation)
//  return scene()->face_selection->center(Space::Viewport);
}

}  // namespace omm
