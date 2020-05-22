#include "objects/path.h"

#include <QObject>
#include "commands/modifypointscommand.h"
#include "properties/boolproperty.h"
#include "properties/optionproperty.h"
#include "scene/scene.h"
#include "geometry/cubics.h"
#include "common.h"
#include "renderers/style.h"

namespace
{

static constexpr auto POSITION_POINTER = "position";
static constexpr auto LEFT_TANGENT_POINTER = "left";
static constexpr auto RIGHT_TANGENT_POINTER = "right";

template<typename Pointer>
auto deserialize_point(omm::AbstractDeserializer& deserializer, const Pointer& pointer)
{
  const auto make_pointer = [&pointer](const auto& top_level_pointer) {
    return omm::Serializable::make_pointer(pointer, top_level_pointer);
  };
  omm::Point point;
  point.position = deserializer.get_vec2f(make_pointer(POSITION_POINTER));
  point.left_tangent = deserializer.get_polarcoordinates(make_pointer(LEFT_TANGENT_POINTER));
  point.right_tangent = deserializer.get_polarcoordinates(make_pointer(RIGHT_TANGENT_POINTER));
  return point;
}

template<typename Pointer>
void serialize_point( omm::AbstractSerializer& serializer, const Pointer& pointer,
                      const omm::Point& point )
{
  const auto make_pointer = [&pointer](const auto& top_level_pointer) {
    return omm::Serializable::make_pointer(pointer, top_level_pointer);
  };
  serializer.set_value(point.position, make_pointer(POSITION_POINTER));
  serializer.set_value(point.left_tangent, make_pointer(LEFT_TANGENT_POINTER));
  serializer.set_value(point.right_tangent, make_pointer(RIGHT_TANGENT_POINTER));
}

}  // namespace

namespace omm
{

class Style;

Path::Path(Scene* scene) : Object(scene)
{
  static const auto category = QObject::tr("path");

  create_property<BoolProperty>(IS_CLOSED_PROPERTY_KEY)
    .set_label(QObject::tr("closed")).set_category(category);

  create_property<OptionProperty>(INTERPOLATION_PROPERTY_KEY)
    .set_options({ QObject::tr("linear"), QObject::tr("smooth"),
                   QObject::tr("bezier") })
    .set_label(QObject::tr("interpolation")).set_category(category);
  update();
}

void Path::draw_object(Painter &renderer, const Style& style, Painter::Options options) const
{
//  renderer.set_style(style, *this, options);
//  renderer.painter->drawPath(m_painter_path);
//  const auto marker_color = style.property(Style::PEN_COLOR_KEY)->value<Color>();
//  const auto width = style.property(Style::PEN_WIDTH_KEY)->value<double>();
//  style.start_marker.draw_marker(renderer, evaluate(0.0).rotated(0.5 * M_PI), marker_color, width);
//  style.end_marker.draw_marker(renderer, evaluate(1.0).rotated(1.5 * M_PI), marker_color, width);
}

BoundingBox Path::bounding_box(const ObjectTransformation &transformation) const
{
  Q_UNUSED(transformation);
  return BoundingBox();
//  if (is_active()) {
//    return BoundingBox((m_painter_path * transformation.to_qtransform()).boundingRect());
//  } else {
//    return BoundingBox();
//  }
}

QString Path::type() const { return TYPE; }

void Path::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  Object::serialize(serializer, root);

//  const auto points_pointer = make_pointer(root, POINTS_POINTER);
//  serializer.start_array(m_points.size(), points_pointer);
//  for (size_t i = 0; i < m_points.size(); ++i) {
//    const auto point_pointer = make_pointer(points_pointer, i);
//    serialize_point(serializer, point_pointer, m_points[i]);
//  }
//  serializer.end_array();
}

void Path::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  Object::deserialize(deserializer, root);

//  const auto points_pointer = make_pointer(root, POINTS_POINTER);
//  size_t n_points = deserializer.array_size(points_pointer);
//  m_points = std::vector<Point>(n_points);
//  for (size_t i = 0; i < n_points; ++i) {
//    const auto point_pointer = make_pointer(points_pointer, i);
//    m_points[i] = deserialize_point(deserializer, point_pointer);
//  }
//  update();
}

void Path::update()
{
//  m_painter_path = Painter::path(m_points, property(IS_CLOSED_PROPERTY_KEY)->value<bool>());
  Object::update();
}

Flag Path::flags() const { return Object::flags() | Flag::IsPathLike; }


}  // namespace omm
