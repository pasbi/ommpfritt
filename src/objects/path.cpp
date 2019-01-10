#include "objects/path.h"

#include <QObject>
#include "commands/modifytangentscommand.h"
#include "properties/boolproperty.h"
#include "properties/optionsproperty.h"
#include "scene/scene.h"

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
  add_property<OptionsProperty>(INTERPOLATION_PROPERTY_KEY)
    .set_options({ "linear", "smooth", "bezier" })
    .set_label(QObject::tr("closed").toStdString())
    .set_category(QObject::tr("path").toStdString())
    .set_post_submit([this](Property&) { this->update_interpolation(); });
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

void Path::make_smooth_tangents(bool constrain_to_selection)
{
  auto modify_point = [=](auto& point, const auto& left, const auto& right, auto& modified_points)
  {
    if (!constrain_to_selection || point.is_selected) {
      const PolarCoordinates l_pc(left.position - point.position);
      const PolarCoordinates r_pc(right.position - point.position);
      const double theta = (l_pc.argument + r_pc.argument) / 2.0;
      const double mag = (l_pc.magnitude + r_pc.magnitude) / 12.0;
      auto copy = point;

      // TODO
      const double d = arma::dot(right.position - point.position, left.position - point.position);
      const double sign = std::copysign(1.0, d);

      copy.left_tangent = PolarCoordinates(theta + M_PI_2, mag);
      copy.right_tangent = PolarCoordinates(theta - M_PI_2, mag);

      // that's a quick hack. If right tangent is closer to left position
      // than left tangent, then swap them.
      // I'm sure there's a more elegant way.
      if ( arma::norm(copy.right_position() - left.position)
         < arma::norm(copy.left_position() - left.position)  )
      {
        copy.left_tangent.swap(copy.right_tangent);
      }
      modified_points.push_back({ point, copy });
    }
  };

  std::list<ModifyTangentsCommand::PointWithAlternative> ps;
  if (property(IS_CLOSED_PROPERTY_KEY).value<bool>()) {
    const auto n = m_points.size();
    modify_point(m_points[0], m_points[n-1], m_points[1], ps);
    modify_point(m_points[n-1], m_points[n-2], m_points[0], ps);
  } else {
    // Do something smart with endpoints
  }

  for (std::size_t i = 1; i < m_points.size() - 1; ++i) {
    modify_point(m_points[i], m_points[i-1], m_points[i+1], ps);
  }

  scene()->submit<ModifyTangentsCommand>(nullptr, ps);
}

void Path::vanish_tangents(bool constrain_to_selection)
{
  std::list<ModifyTangentsCommand::PointWithAlternative> ps;
  for (auto& point : m_points) {
    if (!constrain_to_selection || point.is_selected) {
      auto new_point = point;
      new_point.left_tangent.magnitude = 0;
      new_point.right_tangent.magnitude = 0;
      ps.push_back({ point, new_point });
    }
  }
  scene()->submit<ModifyTangentsCommand>(nullptr, ps);
}

void Path::update_interpolation()
{
  switch (interpolation_mode()) {
  case InterpolationMode::Linear:
    vanish_tangents(false);
    break;
  case InterpolationMode::Smooth:
    make_smooth_tangents(false);
    break;
  case InterpolationMode::Bezier:
    // leave tangents as thet are
    break;
  }
}

Path::InterpolationMode Path::interpolation_mode() const
{
  const auto i = property(INTERPOLATION_PROPERTY_KEY).value<std::size_t>();
  return static_cast<Path::InterpolationMode>(i);
}

void Path::set_interpolation_mode(const InterpolationMode& mode)
{
  const auto i = static_cast<std::size_t>(mode);
  property(INTERPOLATION_PROPERTY_KEY).set(i);
}

}  // namespace omm
