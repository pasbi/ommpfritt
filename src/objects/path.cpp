#include "objects/path.h"

#include <QObject>
#include "commands/modifypointscommand.h"
#include "properties/boolproperty.h"
#include "properties/optionsproperty.h"
#include "scene/scene.h"
#include "geometry/cubic.h"

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
  const auto update_point_tangents = [this](Property&) {
    std::map<Path*, std::map<Point*, Point>> map;
    const auto i_mode = property(INTERPOLATION_PROPERTY_KEY).value<InterpolationMode>();
    map[this] = this->modified_points(false, i_mode);
    this->scene()->submit<ModifyPointsCommand>(map);
  };
  add_property<BoolProperty>(IS_CLOSED_PROPERTY_KEY)
    .set_label(QObject::tr("closed").toStdString())
    .set_category(QObject::tr("path").toStdString());
  add_property<OptionsProperty>(INTERPOLATION_PROPERTY_KEY)
    .set_options({ "linear", "smooth", "bezier" })    // must match Path::InterpolationMode
    .set_label(QObject::tr("interpolation").toStdString())
    .set_category(QObject::tr("path").toStdString())
    .set_post_submit(update_point_tangents).set_pre_submit(update_point_tangents);
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

void Path::deselect_all_points()
{
  for (Point* p : points()) {
    p->is_selected = false;
  }
}

std::map<Point*, Point>
Path::modified_points(const bool constrain_to_selection, InterpolationMode mode)
{
  const auto points = this->points();
  std::map<omm::Point*, omm::Point> map;

  const auto process_point = [constrain_to_selection, mode, &map, points](std::size_t i)
  {
    Point* point = points[i];
    if (!constrain_to_selection || point->is_selected) {
      const auto n = points.size();
      switch (mode) {
      case InterpolationMode::Smooth:
        map[point] = point->smoothed(*points[(i+n-1)%n], *points[(i+n+1)%n]);
        break;
      case InterpolationMode::Linear:
        map[point] = point->nibbed();
        break;
      default:
        break;
      }
    }
  };

  for (std::size_t i = 1; i < points.size()-1; ++i) { process_point(i); }
  if (is_closed()) {
    process_point(0);
    process_point(points.size()-1);
  }
  return map;
}

bool Path::is_closed() const
{
  return this->property(omm::Path::IS_CLOSED_PROPERTY_KEY).value<bool>();
}

OrientedPoint Path::evaluate(const double t)
{
  return Cubics(m_points, is_closed()).evaluate(t);
}

double Path::path_length()
{
  return Cubics(m_points, is_closed()).length();
}

}  // namespace omm
