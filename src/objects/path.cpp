#include "objects/path.h"

#include "commands/modifypointscommand.h"
#include "common.h"
#include "properties/boolproperty.h"
#include "properties/optionproperty.h"
#include "renderers/style.h"
#include "scene/scene.h"
#include "objects/pathpoint.h"
#include "objects/segment.h"
#include <QObject>
#include "scene/mailbox.h"

namespace
{

using namespace omm;

auto copy(const std::deque<std::unique_ptr<Segment>>& vs, Path* path)
{
  std::decay_t<decltype(vs)> copy;
  for (auto&& v : vs) {
    copy.emplace_back(std::make_unique<omm::Segment>(*v, path));
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

  create_property<OptionProperty>(INTERPOLATION_PROPERTY_KEY)
      .set_options({QObject::tr("linear"), QObject::tr("smooth"), QObject::tr("bezier")})
      .set_label(QObject::tr("interpolation"))
      .set_category(category);
  Path::update();

  connect(&scene->mail_box(), &MailBox::transformation_changed, this, [this](const Object& o) {
    if (&o == this) {
      update_joined_points_geometry();
    }
  });
}

Path::Path(const Path& other)
  : Object(other)
  , m_segments(copy(other.m_segments, this))
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
    Segment& segment = *m_segments.emplace_back(std::make_unique<Segment>(this));
    segment.deserialize(deserializer, make_pointer(segments_pointer, i));
  }
  update();
}

void Path::set(const Geom::PathVector& paths)
{
  update();
  for (const auto& path : paths) {
    m_segments.push_back(std::make_unique<Segment>(path, this));
  }
}

PathPoint& Path::point_at_index(std::size_t index) const
{
  for (Segment* segment : segments()) {
    if (index < segment->size()) {
      return segment->at(index);
    } else {
      index -= segment->size();
    }
  }
  throw std::runtime_error{"Index out of bounds."};
}

std::size_t Path::point_count() const
{
  return std::accumulate(cbegin(m_segments), cend(m_segments), 0, [](std::size_t n, auto&& segment) {
    return n + segment->size();
  });
}

void Path::on_property_value_changed(Property* property)
{
  if (pmatch(property, {INTERPOLATION_PROPERTY_KEY})) {
    update();
  }
  Object::on_property_value_changed(property);
}

Geom::PathVector Path::paths() const
{
  const auto interpolation = property(INTERPOLATION_PROPERTY_KEY)->value<InterpolationMode>();

  Geom::PathVector paths;
  for (auto&& segment : m_segments) {
    paths.push_back(segment->to_geom_path(interpolation));
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

Segment* Path::find_segment(const PathPoint& point) const
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

std::unique_ptr<Segment> Path::remove_segment(const Segment& segment)
{
  const auto it = std::find_if(m_segments.begin(), m_segments.end(), [&segment](const auto& s_ptr) {
    return &segment == s_ptr.get();
  });
  std::unique_ptr<Segment> extracted_segment;
  std::swap(extracted_segment, *it);
  m_segments.erase(it);
  return extracted_segment;
}

std::deque<PathPoint*> Path::points() const
{
  std::deque<PathPoint*> points;
  for (const auto& segment : m_segments) {
    const auto& ps = segment->points();
    points.insert(points.end(), ps.begin(), ps.end());
  }
  return points;
}

std::deque<PathPoint*> Path::selected_points() const
{
  return ::filter_if(points(), std::mem_fn(&PathPoint::is_selected));
}

void Path::deselect_all_points() const
{
  for (auto* point : points()) {
    point->set_selected(false);
  }
}

void Path::update_joined_points_geometry() const
{
  std::set<Path*> updated_paths;
  for (auto* point : points()) {
    for (auto* buddy : point->joined_points()) {
      if (buddy != point && buddy->path() != this) {
        updated_paths.insert(buddy->path());
        buddy->set_geometry(point->compute_joined_point_geometry(*buddy));
      }
    }
  }
  for (auto* path : updated_paths) {
    path->update();
  }
}

}  // namespace omm
