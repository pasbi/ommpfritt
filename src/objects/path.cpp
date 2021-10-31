#include "objects/path.h"

#include "commands/modifypointscommand.h"
#include "common.h"
#include "properties/boolproperty.h"
#include "properties/optionproperty.h"
#include "renderers/style.h"
#include "scene/scene.h"
#include "objects/segment.h"
#include <QObject>

namespace
{

auto copy(const std::deque<std::unique_ptr<omm::Segment>>& vs)
{
  std::decay_t<decltype(vs)> copy;
  for (auto&& v : vs) {
    copy.emplace_back(std::make_unique<omm::Segment>(*v));
  }
  return copy;
}

}  // namespace

namespace omm
{
class Style;

Path::Path(Scene* scene) : Object(scene)
{
  static const auto category = QObject::tr("path");

  create_property<BoolProperty>(IS_CLOSED_PROPERTY_KEY)
      .set_label(QObject::tr("closed"))
      .set_category(category);

  create_property<OptionProperty>(INTERPOLATION_PROPERTY_KEY)
      .set_options({QObject::tr("linear"), QObject::tr("smooth"), QObject::tr("bezier")})
      .set_label(QObject::tr("interpolation"))
      .set_category(category);
  Path::update();
}

Path::Path(const Path& other)
  : Object(other)
  , m_segments(copy(other.m_segments))
{
}

Path::~Path() = default;

QString Path::type() const
{
  return TYPE;
}

void Path::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  Object::serialize(serializer, root);

  const auto segments_pointer = make_pointer(root, SEGMENTS_POINTER);
  serializer.start_array(m_segments.size(), segments_pointer);
  for (std::size_t i = 0; i < m_segments.size(); ++i) {
    if (m_segments.empty()) {
      LWARNING << "Ignoring empty sub-path.";
    } else {
      m_segments[i]->serialize(serializer, make_pointer(segments_pointer, i));
    }
  }
  serializer.end_array();
}

void Path::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  Object::deserialize(deserializer, root);

  const auto segments_pointer = make_pointer(root, SEGMENTS_POINTER);
  const std::size_t n_paths = deserializer.array_size(segments_pointer);
  m_segments.clear();
  for (std::size_t i = 0; i < n_paths; ++i) {
    Segment& segment = *m_segments.emplace_back(std::make_unique<Segment>());
    segment.deserialize(deserializer, make_pointer(segments_pointer, i));
  }
  update();
}

void Path::update()
{
  painter_path.invalidate();
  geom_paths.invalidate();
  Object::update();
}

bool Path::is_closed() const
{
  return property(IS_CLOSED_PROPERTY_KEY)->value<bool>();
}

void Path::set(const Geom::PathVector& paths)
{
  update();
  const auto is_closed = this->is_closed();
  for (const auto& path : paths) {
    m_segments.push_back(std::make_unique<Segment>(path, is_closed));
  }
}

std::size_t Path::point_count() const
{
  return std::accumulate(cbegin(m_segments), cend(m_segments), 0, [](std::size_t n, auto&& segment) {
    return n + segment->size();
  });
}

void Path::on_property_value_changed(Property* property)
{
  if (pmatch(property, {INTERPOLATION_PROPERTY_KEY, IS_CLOSED_PROPERTY_KEY})) {
    update();
  }
  Object::on_property_value_changed(property);
}

Geom::PathVector Path::paths() const
{
  const auto interpolation = property(INTERPOLATION_PROPERTY_KEY)->value<InterpolationMode>();
  const auto is_closed = this->is_closed();

  Geom::PathVector paths;
  for (auto&& segment : m_segments) {
    paths.push_back(segment->to_geom_path(interpolation, is_closed));
  }
  return paths;
}

Flag Path::flags() const
{
  return Flag::None;
}

std::deque<Segment*> Path::segments() const
{
  return ::transform<Segment*>(m_segments, std::mem_fn(&std::unique_ptr<Segment>::get));
}

Segment* Path::find_segment(const Point& point) const
{
  for (auto&& segment : m_segments) {
    if (segment->contains(point)) {
      return segment.get();
    }
  }
  return nullptr;
}

Segment& Path::add_segment(std::unique_ptr<Segment>&& segment)
{
  return *m_segments.emplace_back(std::move(segment));
}

std::deque<Point*> Path::points() const
{
  std::deque<Point*> points;
  for (const auto& segment : m_segments) {
    const auto& ps = segment->points();
    points.insert(points.end(), ps.begin(), ps.end());
  }
  return points;
}

std::deque<Point*> Path::selected_points() const
{
  return ::filter_if(points(), std::mem_fn(&Point::is_selected));
}

}  // namespace omm
