#include "objects/path.h"

#include <QObject>
#include "commands/modifypointscommand.h"
#include "properties/boolproperty.h"
#include "properties/optionsproperty.h"
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
  static const auto category = QObject::tr("path").toStdString();

  create_property<BoolProperty>(IS_CLOSED_PROPERTY_KEY)
    .set_label(QObject::tr("closed").toStdString()).set_category(category);

  create_property<OptionsProperty>(INTERPOLATION_PROPERTY_KEY)
    .set_options({ QObject::tr("linear").toStdString(), QObject::tr("smooth").toStdString(),
                   QObject::tr("bezier").toStdString() })
    .set_label(QObject::tr("interpolation").toStdString()).set_category(category);
  update();
}

void Path::draw_object(Painter &renderer, const Style& style) const
{
  const auto triangulation_style = ContourStyle(Colors::BLACK, 0.5);
  const auto marked_triangulation_style = ContourStyle(Colors::GREEN, 2.0);
  renderer.set_style(style);
  renderer.painter->drawPath(m_painter_path);
  const auto marker_color = style.property(Style::PEN_COLOR_KEY)->value<Color>();
  const auto width = style.property(Style::PEN_WIDTH_KEY)->value<double>();
  style.start_marker.draw_marker(renderer, evaluate(0.0).rotated(0.5 * M_PI), marker_color, width);
  style.end_marker.draw_marker(renderer, evaluate(1.0).rotated(1.5 * M_PI), marker_color, width);
}

BoundingBox Path::bounding_box(const ObjectTransformation &transformation) const
{
  if (is_active()) {
    return BoundingBox((m_painter_path * transformation.to_qtransform()).boundingRect());
  } else {
    return BoundingBox();
  }
}

std::string Path::type() const { return TYPE; }
std::unique_ptr<Object> Path::clone() const { return std::make_unique<Path>(*this); }

void Path::set_points(const std::vector<Point>& points)
{
  m_points = points;
  update();
}

std::vector<Point> Path::points() const { return m_points; }

std::vector<Point*> Path::points_ref()
{
  return ::transform<Point*>(m_points, [](Point& p) { return &p; });
}

Point &Path::point(std::size_t i) { return m_points[i]; }
const Point &Path::point(std::size_t i) const { return m_points[i]; }

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
  update();
}

void Path::deselect_all_points()
{
  for (Point& p : m_points) {
    p.is_selected = false;
  }
}

std::map<Point*, Point>
Path::modified_points(const bool constrain_to_selection, InterpolationMode mode)
{
  const auto points = this->points();
  std::map<omm::Point*, omm::Point> map;

  for (std::size_t i = 0; i < points.size(); ++i) {
    if (!constrain_to_selection || m_points[i].is_selected) {
      switch (mode) {
      case InterpolationMode::Smooth:
        map[&m_points[i]] = smoothed(i);
        break;
      case InterpolationMode::Linear:
        map[&m_points[i]] = m_points[i].nibbed();
        break;
      case InterpolationMode::Bezier:
        break;
      }
    }
  }
  return map;
}

Point Path::smoothed(const std::size_t& i) const
{
  Vec2f left, right;
  const std::size_t n = m_points.size();
  if (i == 0) {
   left = is_closed() ? m_points[n-1].position : m_points[0].position;
   right = m_points[1].position;
  } else if (i == n-1) {
   left = m_points[n-2].position;
   right = is_closed() ? m_points[0].position : m_points[n-1].position;
  } else {
   left = m_points[i-1].position;
   right = m_points[i+1].position;
  }
  return m_points[i].smoothed(left, right);
}

bool Path::contains(const Vec2f &pos) const { return cubics().contains(pos); }

void Path::update()
{
  m_painter_path = Painter::path(m_points, property(IS_CLOSED_PROPERTY_KEY)->value<bool>());
  Object::update();
}

void Path::on_property_value_changed(Property *property)
{
  if (property == this->property(INTERPOLATION_PROPERTY_KEY)) {
    std::map<Path*, std::map<Point*, Point>> map;
    const auto i_mode = property->value<InterpolationMode>();
    map[this] = this->modified_points(false, i_mode);
    ModifyPointsCommand(map).redo();
  } else if (   property == this->property(IS_CLOSED_PROPERTY_KEY)
             || property == this->property(INTERPOLATION_PROPERTY_KEY))
  {
    update();
  } else {
    Object::on_property_value_changed(property);
  }
}

bool Path::is_closed() const
{
  return this->property(omm::Path::IS_CLOSED_PROPERTY_KEY)->value<bool>();
}

Point Path::evaluate(const double t) const
{
  return cubics().evaluate(t);
}

Cubics Path::cubics() const { return Cubics(m_points, is_closed()); }

double Path::path_length() const
{
  return cubics().length();
}

std::vector<std::size_t> Path::selected_points() const
{
  std::list<std::size_t> selection;
  for (std::size_t i = 0; i < m_points.size(); ++i) {
    if (m_points[i].is_selected) { selection.push_back(i); }
  }
  return std::vector(selection.begin(), selection.end());
}

std::vector<std::size_t> Path::add_points(const PointSequence& sequence)
{
  auto i = std::next(m_points.begin(), static_cast<int>(sequence.position));

  m_points.insert(i, sequence.sequence.begin(), sequence.sequence.end());

  std::vector<std::size_t> points;
  const auto n = sequence.sequence.size();
  points.reserve(n);
  for (std::size_t j = 0; j < n; ++j) {
    points.push_back(sequence.position);
  }
  return points;
}

std::vector<std::size_t> Path::add_points(std::vector<PointSequence> sequences)
{
  std::sort(sequences.begin(), sequences.end(), [](const auto& s1, const auto& s2) {
    assert(s1.position != s2.position); // sequences shall be merged.
    return s1.position > s2.position;
  });

  std::list<std::size_t> indices;
  for (const auto& sequence : sequences) {
    auto ii = add_points(sequence);
    indices.insert(indices.end(), ii.begin(), ii.end());
  }

  update();
  return std::vector(indices.begin(), indices.end());
}

std::vector<Path::PointSequence> Path::remove_points(std::vector<std::size_t> indices)
{
  std::sort(indices.begin(), indices.end());
  std::list<Path::PointSequence> sequences;

  const auto is_continuous = [&sequences](const std::size_t i) {
    if (sequences.empty()) {
      return false;
    } else {
      return sequences.back().position == i;
    }
  };

  for (std::size_t i : indices) {
    if (is_continuous(i)) {
      sequences.back().sequence.push_back(m_points[i]);
    } else {
      sequences.push_back(Path::PointSequence(i, std::list{m_points[i]} ));
    }
    m_points.erase(std::next(m_points.begin(), static_cast<int>(i)));
  }

  update();
  return std::vector(sequences.begin(), sequences.end());
}

AbstractPropertyOwner::Flag Path::flags() const { return Object::flags() | Flag::IsPathLike; }

void Path::set_global_axis_transformation(const ObjectTransformation& global_transformation,
                                           Space space )
{
  const auto td = global_transformation.inverted().apply(this->global_transformation(space));
  Object::set_global_axis_transformation(global_transformation, space);
  for (auto& point : m_points) {
    point = td.apply(point);
  }
}

std::vector<double> Path::cut(const Vec2f& c_start, const Vec2f& c_end)
{
  const auto gti = global_transformation(Space::Viewport).inverted();
  return cubics().cut(gti.apply_to_position(c_start), gti.apply_to_position(c_end));
}

std::vector<Path::PointSequence> Path::get_point_sequences(const std::vector<double> &ts) const
{
  const auto cubics = this->cubics();
  std::map<std::size_t, std::list<double>> sequences_t;
  for (double t : ts) {
    const auto [segment_i, segment_t] = cubics.path_to_segment(t);
    sequences_t[segment_i].push_back(segment_t);
  }

  const auto f = [cubics](auto i_ts) {
    auto [segment_i, sequence_t] = i_ts;
    sequence_t.sort();
    const auto f = [segment_i=segment_i, cubics](const double segment_t) {
      return cubics.segment(segment_i).evaluate(segment_t);
    };
    return PointSequence(segment_i + 1, ::transform<Point>(sequence_t, f));
  };

  auto sequences = ::transform<PointSequence, std::vector>(sequences_t, f);
  std::sort(sequences.begin(), sequences.end(), [](const auto& a, const auto& b) {
    return a.position > b.position;
  });
  return sequences;
}

void Path::update_tangents()
{
  switch (property(INTERPOLATION_PROPERTY_KEY)->value<InterpolationMode>()) {
  case InterpolationMode::Bezier: break;
  case InterpolationMode::Linear:
    for (std::size_t i = 0; i < m_points.size(); ++i) {
      m_points[i] = m_points[i].nibbed();
    }
    break;
  case InterpolationMode::Smooth:
    for (std::size_t i = 0; i < m_points.size(); ++i) {
      m_points[i] = smoothed(i);
    }
    break;
  }
}

Object::PathUniquePtr Path::outline(const double t) const
{
  auto outline = std::make_unique<Path>(scene());
  outline->set_points(Point::offset(t, m_points, is_closed()));
  outline->property(IS_CLOSED_PROPERTY_KEY)->set(is_closed());
  return Object::PathUniquePtr(outline.release());
}

Path::PointSequence::PointSequence(int position) : position(position) {}

Path::PointSequence::PointSequence(int position, const std::initializer_list<Point> &points)
  : position(position), sequence(points.begin(), points.end()) {}

Path::PointSequence::PointSequence(int position, const std::list<Point> &points)
  : position(position), sequence(points) {}

Path::PointSequence::PointSequence(int position, const std::vector<Point> &points)
  : position(position), sequence(points.begin(), points.end()) {}

}  // namespace omm
