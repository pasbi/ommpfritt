#include "tools/selectfacestool.h"
#include "handles/facehandle.h"
#include "objects/pathobject.h"
#include "path/face.h"
#include "path/pathvector.h"
#include "scene/faceselection.h"
#include "scene/scene.h"

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

void SelectFacesTool::reset()
{
  clear();
  make_handles();
}

void SelectFacesTool::make_handles()
{
  for (auto* path_object : scene()->item_selection<PathObject>()) {
    for (const auto& face : path_object->faces()) {
      auto handle = std::make_unique<FaceHandle>(*this, *path_object, face);
      push_handle(std::move(handle));
    }
  }
}

}  // namespace omm
