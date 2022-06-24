#include "objects/pathobject.h"

#include "commands/modifypointscommand.h"
#include "common.h"
#include "path/path.h"
#include "path/pathgeometry.h"
#include "path/pathvector.h"
#include "path/pathvectorgeometry.h"
#include "properties/boolproperty.h"
#include "properties/optionproperty.h"
#include "renderers/style.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include <QObject>

#ifdef DRAW_POINT_IDS
#include "path/pathpoint.h"
#include "renderers/painter.h"
#include <QPainter>
#endif  // DRAW_POINT_IDS


namespace omm
{

class Style;

PathObject::PathObject(Scene* scene, std::unique_ptr<PathVector> path_vector)
  : Object(scene)
  , m_path_vector(std::move(path_vector))
{
  static const auto category = QObject::tr("path");

  create_property<OptionProperty>(INTERPOLATION_PROPERTY_KEY)
      .set_options({QObject::tr("linear"), QObject::tr("smooth"), QObject::tr("bezier")})
      .set_label(QObject::tr("interpolation"))
      .set_category(category);
  PathObject::update();
}

PathObject::PathObject(Scene* scene, const PathVectorGeometry& geometry)
  : PathObject(scene, std::make_unique<PathVector>(geometry, this))
{
}

PathObject::PathObject(Scene* scene)
  : PathObject(scene, std::make_unique<PathVector>(this))
{
}

PathObject::PathObject(const PathObject& other)
  : Object(other)
  , m_path_vector(copy_unique_ptr(other.m_path_vector, this))
{
}

PathObject::~PathObject() = default;

QString PathObject::type() const
{
  return TYPE;
}

void PathObject::serialize(serialization::SerializerWorker& worker) const
{
  Object::serialize(worker);
  m_path_vector->serialize(*worker.sub(PATH_VECTOR_POINTER));
}

void PathObject::deserialize(serialization::DeserializerWorker& worker)
{
  Object::deserialize(worker);
  m_path_vector->deserialize(*worker.sub(PATH_VECTOR_POINTER));
  update();
}

void PathObject::on_property_value_changed(Property* property)
{
  if (pmatch(property, {INTERPOLATION_PROPERTY_KEY})) {
    update();
  }
  Object::on_property_value_changed(property);
}

Flag PathObject::flags() const
{
  return Flag::None;
}

const PathVector& PathObject::path_vector() const
{
  return *m_path_vector;
}

PathVector& PathObject::path_vector()
{
  return *m_path_vector;
}

PathVectorGeometry PathObject::compute_geometry() const
{
  const auto interpolation = property(INTERPOLATION_PROPERTY_KEY)->value<InterpolationMode>();

  auto paths = m_path_vector->geometry().paths();
  for (auto& path : paths) {
    path.set_interpolation(interpolation);
  }
  return PathVectorGeometry{std::move(paths)};
}

void PathObject::set_face_selected(const Face& face, bool s)
{
  Q_UNUSED(face)
  Q_UNUSED(s)
}

bool PathObject::is_face_selected(const Face& face) const
{
  Q_UNUSED(face)
  return false;
}

#ifdef DRAW_POINT_IDS
void PathObject::draw_object(Painter& renderer, const Style& style, const PainterOptions& options) const
{
  Object::draw_object(renderer, style, options);
  renderer.painter->save();
  renderer.painter->setPen(Qt::white);
  path_vector().draw_point_ids(*renderer.painter);
  renderer.painter->restore();
}
#endif  // DRAW_POINT_IDS

}  // namespace omm
