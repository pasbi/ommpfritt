#include "objects/pathobject.h"

#include "commands/modifypointscommand.h"
#include "common.h"
#include "path/path.h"
#include "path/pathvector.h"
#include "properties/boolproperty.h"
#include "properties/optionproperty.h"
#include "renderers/style.h"
#include "scene/disjointpathpointsetforest.h"
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

  if (scene != nullptr) {
    connect(&scene->mail_box(), &MailBox::transformation_changed, this, [this](const Object& o) {
      if (&o == this) {
        geometry().update_joined_points_geometry();
      }
    });
  }
}

PathObject::PathObject(Scene* scene, const PathVector& path_vector)
  : PathObject(scene, std::make_unique<PathVector>(path_vector, this))
{
}

PathObject::PathObject(Scene* scene)
  : PathObject(scene, std::make_unique<PathVector>(this))
{
  if (const auto* const scene = this->scene(); scene != nullptr) {
    m_path_vector->share_joined_points(scene->joined_points());
  }
}

PathObject::PathObject(const PathObject& other)
  : Object(other)
  , m_path_vector(copy_unique_ptr(other.m_path_vector, this))
{
  if (const auto*  const scene = this->scene(); scene != nullptr && other.path_vector().joined_points_shared()) {
    m_path_vector->share_joined_points(scene->joined_points());
  }
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

const PathVector& PathObject::geometry() const
{
  return *m_path_vector;
}

PathVector& PathObject::geometry()
{
  return *m_path_vector;
}

PathVector PathObject::compute_path_vector() const
{
  const auto interpolation = property(INTERPOLATION_PROPERTY_KEY)->value<InterpolationMode>();

  PathVector pv{*m_path_vector};
  for (auto* path : pv.paths()) {
    path->set_interpolation(interpolation);
  }
  return pv;
}

#ifdef DRAW_POINT_IDS
void PathObject::draw_object(Painter& renderer, const Style& style, const PainterOptions& options) const
{
  Object::draw_object(renderer, style, options);
  renderer.painter->save();
  renderer.painter->setPen(Qt::white);
  for (const auto* point : path_vector().points()) {
    static constexpr QPointF offset{10.0, 10.0};
    renderer.painter->drawText(point->geometry().position().to_pointf() + offset, point->debug_id());
  }
  renderer.painter->restore();
}
#endif  // DRAW_POINT_IDS

}  // namespace omm
