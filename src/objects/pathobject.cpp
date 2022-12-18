#include "objects/pathobject.h"

#include "common.h"
#include "path/pathvector.h"
#include "properties/optionproperty.h"
#include "scene/scene.h"
#include <QObject>

#if DRAW_POINT_IDS
#  include "path/pathpoint.h"
#  include "renderers/painter.h"
#  include <QPainter>
#endif  // DRAW_POINT_IDS


namespace omm
{

class Style;

PathObject::PathObject(Scene* scene, const PathVector& path_vector)
    : PathObject(scene, std::make_unique<PathVector>(path_vector, this))
{

}

PathObject::PathObject(Scene* scene)
    : PathObject(scene, std::make_unique<PathVector>(this))
{
}

PathObject::PathObject(const PathObject& other)
  : Object(other), m_path_vector(std::make_unique<PathVector>(*other.m_path_vector, this))
{
}

PathObject::PathObject(Scene* scene, std::unique_ptr<PathVector> path_vector)
  : Object(scene)
  , m_path_vector(std::move(path_vector))
{
  m_path_vector->set_path_object(this);
  static const auto category = QObject::tr("path");

  create_property<OptionProperty>(INTERPOLATION_PROPERTY_KEY)
      .set_options({QObject::tr("linear"), QObject::tr("smooth"), QObject::tr("bezier")})
      .set_label(QObject::tr("interpolation"))
      .set_category(category);
  PathObject::update();
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

std::unique_ptr<PathVector> PathObject::compute_geometry() const
{
  return std::make_unique<PathVector>(*m_path_vector);
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

#if DRAW_POINT_IDS
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
