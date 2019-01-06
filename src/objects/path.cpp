#include "objects/path.h"

#include <QObject>

#include "properties/boolproperty.h"

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
  point.position = deserializer.get_vec2(make_pointer(POSITION_POINTER));
  point.left_tangent = deserializer.get_polar_coordinates(make_pointer(LEFT_TANGENT_POINTER));
  point.right_tangent = deserializer.get_polar_coordinates(make_pointer(RIGHT_TANGENT_POINTER));
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
  add_property<BoolProperty>(IS_CLOSED_PROPERTY_KEY)
    .set_label(QObject::tr("closed").toStdString())
    .set_category(QObject::tr("path").toStdString());
}

void Path::render(AbstractRenderer& renderer, const Style& style)
{
  renderer.draw_spline(m_points, style, property("closed").value<bool>());
}

BoundingBox Path::bounding_box()
{
  return BoundingBox(); // TODO
}

std::string Path::type() const
{
  return TYPE;
}

std::unique_ptr<Object> Path::clone() const
{
  return std::make_unique<Path>(*this);
}

void Path::set_points(const std::vector<Point>& points)
{
  m_points = points;
}

std::vector<Point*> Path::points()
{
  return ::transform<Point*>(m_points, [](Point& p) { return &p; });
}

void Path::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  Object::serialize(serializer, root);

  const auto points_pointer = make_pointer(root, POINTS_POINTER);
  serializer.start_array(m_points.size(), points_pointer);
  for (size_t i = 0; i < m_points.size(); ++i) {
    const auto point_pointer = make_pointer(points_pointer, i);
    serialize_point(serializer, point_pointer, m_points[i]);
  }
  serializer.end_array();
}

void Path::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  Object::deserialize(deserializer, root);

  const auto points_pointer = make_pointer(root, POINTS_POINTER);
  size_t n_points = deserializer.array_size(points_pointer);
  m_points = std::vector<Point>(n_points);
  for (size_t i = 0; i < n_points; ++i) {
    const auto point_pointer = make_pointer(points_pointer, i);
    m_points[i] = deserialize_point(deserializer, point_pointer);
  }
}

}  // namespace omm
