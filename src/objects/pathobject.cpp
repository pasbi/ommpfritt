#include "objects/pathobject.h"

#include "commands/modifypointscommand.h"
#include "common.h"
#include "properties/boolproperty.h"
#include "properties/optionproperty.h"
#include "renderers/style.h"
#include "scene/scene.h"
#include "path/path.h"
#include "scene/mailbox.h"
#include "path/pathvector.h"
#include <QObject>

namespace omm
{

class Style;

PathObject::PathObject(Scene* scene)
  : Object(scene)
  , m_path_vector(std::make_unique<PathVector>(this))
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

    m_path_vector->share_join_points(scene->joined_points());
  }
}

PathObject::PathObject(const PathObject& other)
  : Object(other)
  , m_path_vector(copy_unique_ptr(other.m_path_vector, this))
{
  if (auto* scene = this->scene(); scene != nullptr) {
    m_path_vector->share_join_points(scene->joined_points());
  }
}

PathObject::~PathObject() = default;

QString PathObject::type() const
{
  return TYPE;
}

void PathObject::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  Object::serialize(serializer, root);
  m_path_vector->serialize(serializer, make_pointer(root, PATH_VECTOR_POINTER));
}

void PathObject::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  Object::deserialize(deserializer, root);
  m_path_vector->deserialize(deserializer, make_pointer(root, PATH_VECTOR_POINTER));
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
  PathVector pv;
  pv.set(m_path_vector->to_geom(interpolation));
  return pv;
}

}  // namespace omm
