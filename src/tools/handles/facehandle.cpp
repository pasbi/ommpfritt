#include "tools/handles/facehandle.h"
#include "path/edge.h"
#include <QPainter>
#include "objects/pathobject.h"
#include "scene/faceselection.h"
#include "scene/scene.h"

namespace omm
{

FaceHandle::FaceHandle(Tool& tool, PathObject& path_object, const Face& face)
    : AbstractSelectHandle(tool)
    , m_path_object(path_object)
    , m_face(face)
    , m_path(face.to_painter_path())
{
}

bool FaceHandle::contains_global(const Vec2f& point) const
{
  const auto p = transformation().inverted().apply_to_position(point);
  return m_face.contains(p);
}

void FaceHandle::draw(QPainter& painter) const
{
  painter.save();
  painter.setTransform(transformation().to_qtransform());
  const auto status = is_selected() ? HandleStatus::Active : this->status();
  painter.setBrush(ui_color(status, "face"));
  painter.drawPath(m_path);
  painter.restore();
}

ObjectTransformation FaceHandle::transformation() const
{
  return m_path_object.global_transformation(Space::Viewport);
}

bool FaceHandle::is_selected() const
{
  return tool.scene()->face_selection->is_selected(m_face);
}

void FaceHandle::set_selected(bool selected)
{
  tool.scene()->face_selection->set_selected(m_face, selected);
}

void FaceHandle::clear()
{
  return tool.scene()->face_selection->clear();
}

}  // namespace omm
