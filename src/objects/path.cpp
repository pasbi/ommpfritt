#include "objects/path.h"

#include <QObject>
#include "commands/modifypointscommand.h"
#include "properties/boolproperty.h"
#include "properties/optionsproperty.h"
#include "scene/scene.h"
#include "geometry/cubic.h"
#include "common.h"

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

std::vector<std::size_t> Path::selected_points() const
{
  std::list<std::size_t> selection;
  for (std::size_t i = 0; i < m_points.size(); ++i) {
    if (m_points[i].is_selected) { selection.push_back(i); }
  }
  return std::vector(selection.begin(), selection.end());
}

std::vector<Path::PointSequence> Path::remove_points(std::vector<std::size_t> indices)
{
  using ::operator<<;
  // `points` may have holes, but must be ordered.
  std::list<PointSequence> sequences;
  if (indices.size() > 0) {
    sequences.push_back(PointSequence{});
    std::size_t i = 0;
    bool fresh = true;
    for (std::size_t j = 0; j < m_points.size(); ++j) {
      if (i == indices.size()) {
        break;
      } else if (j == indices[i]) {
        sequences.back().sequence.push_back(m_points[j]);
        i++;
        fresh = false;
      } else {
        if (!fresh) {
          sequences.push_back(PointSequence{});
          fresh = true;
        }
        sequences.back().position = j+1;
      }
    }
    assert(i == indices.size()); // otherwise, indices was not ordered.
    if (fresh) {  sequences.pop_back(); }
  }

  std::vector<Point> new_points;
  new_points.reserve(m_points.size() - indices.size());
  std::size_t j = 0;
  for (std::size_t i = 0; i < m_points.size(); ++i) {
    if (j >= indices.size() || i != indices[j]) {
      new_points.push_back(m_points[i]);
    } else {
      j++;
    }
  }
  assert(new_points.size() == m_points.size() - indices.size());
  m_points = new_points;

  scene()->tool_box.active_tool().on_scene_changed();
  return std::vector<Path::PointSequence>(sequences.begin(), sequences.end());
}

std::vector<std::size_t> Path::add_points(const PointSequence& sequence)
{
  auto i = std::next(m_points.begin(), sequence.position);
  const auto n = sequence.sequence.size();

  m_points.insert(i, sequence.sequence.begin(), sequence.sequence.end());

  std::vector<std::size_t> points;
  points.reserve(n);
  for (std::size_t j = 0; j < n; ++j) {
    points.push_back(sequence.position + j);
  }
  return points;
}

std::vector<std::size_t> Path::add_points(const std::vector<PointSequence>& sequences)
{
  using ::operator<<;
  std::size_t last_pos = 0;
  std::list<std::size_t> points;
  for (std::size_t i = 0; i < sequences.size(); ++i) {
    const auto pos = sequences[i].position;
    // sequences must be separated by at least one item.
    // subsequent sequences are acutually not a problem, however, they are expected to be merged
    // into a single sequence. Sequences must not interleave because it produces unintuive effects.
    assert(i == 0 || pos > sequences[i-1].position + sequences[i-1].sequence.size());
    last_pos = pos;
    const auto ps = add_points(sequences[i]);
    std::copy(ps.begin(), ps.end(), std::back_inserter(points));
  }
  if (scene()) {
    // scene() == nullptr should happen only in unit-test context.
    scene()->tool_box.active_tool().on_scene_changed();
  }
  return std::vector(points.begin(), points.end());
}



}  // namespace omm
