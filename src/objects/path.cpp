#include "objects/path.h"

#include <QObject>
#include "commands/modifypointscommand.h"
#include "properties/boolproperty.h"
#include "properties/optionproperty.h"
#include "scene/scene.h"
#include "common.h"
#include "renderers/style.h"

namespace
{

template<typename Iterator>
auto iterator_to_tuple(const Iterator& it)
{
  return std::tuple{it.path, it.segment, it.point};
}

}  // namespace

namespace omm
{

class Style;

Path::Path(Scene* scene)
  : AbstractPath(scene)
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
  AbstractPath::draw_object(renderer, style, options);
  const auto marker_color = style.property(Style::PEN_COLOR_KEY)->value<Color>();
  const auto width = style.property(Style::PEN_WIDTH_KEY)->value<double>();
  style.start_marker.draw_marker(renderer, pos(0.0).rotated(0.5 * M_PI), marker_color, width);
  style.end_marker.draw_marker(renderer, pos(1.0).rotated(1.5 * M_PI), marker_color, width);
}

BoundingBox Path::bounding_box(const ObjectTransformation &transformation) const
{
  Q_UNUSED(transformation);
  return BoundingBox();
  if (is_active()) {
    return BoundingBox((painter_path() * transformation.to_qtransform()).boundingRect());
  } else {
    return BoundingBox();
  }
}

QString Path::type() const { return TYPE; }

void Path::serialize(AbstractSerializer& serializer, const Pointer& root) const
{
  Object::serialize(serializer, root);

  const auto subpath_ptr = make_pointer(root, SUBPATH_POINTER);
  serializer.start_array(segments.size(), subpath_ptr);
  for (std::size_t i = 0; i < segments.size(); ++i) {
    if (segments.size() == 0) {
      LWARNING << "Ignoring empty sub-path.";
    } else {
      const auto pts_ptr = make_pointer(subpath_ptr, i);
      serializer.start_array(segments.size(), pts_ptr);
      std::size_t j = 0;
      for (auto&& point : segments[i]) {
        point.serialize(serializer, make_pointer(pts_ptr, j));
        j += 1;
        serializer.end_array();
      }
    }
  }
  serializer.end_array();
}

void Path::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  Object::deserialize(deserializer, root);

  const auto subpath_ptr = make_pointer(root, SUBPATH_POINTER);
  const std::size_t n_paths = deserializer.array_size(subpath_ptr);
  segments.clear();
  segments.reserve(n_paths);
  for (size_t i = 0; i < n_paths; ++i) {
    const auto pts_ptr = make_pointer(subpath_ptr, i);
    const std::size_t n_points = deserializer.array_size(pts_ptr);
    if (n_points == 0) {
      throw AbstractDeserializer::DeserializeError("Empty sub-paths are not allowed.");
    }
    segments.push_back({});
    for (size_t j = 0; j < n_points; ++j) {
      Point p;
      p.deserialize(deserializer, make_pointer(pts_ptr, j));
      segments[i].push_back(p);
    }
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
  const auto path_to_segment = [is_closed=this->is_closed()](const Geom::Path& path) {
    Segment segment;
    segment.reserve(path.size_default() + 1);
    for (auto&& curve : path) {
      const auto& c = dynamic_cast<const Geom::CubicBezier&>(curve);
      if (segment.empty()) {
        segment.push_back(Point({c[0].x(), c[0].y()}));
      }
      segment.back().right_tangent = PolarCoordinates(c[1].x(), c[1].y());
      segment.push_back(Point({c[3].x(), c[3].y()}));
      segment.back().left_tangent = PolarCoordinates(c[2].x(), c[2].y());
    }
    if (is_closed) {
      segment.front().left_tangent = segment.back().left_tangent;
      segment.back().right_tangent = segment.front().right_tangent;
    }
    return segment;
  };
  segments = ::transform<Segment, std::vector>(paths, path_to_segment);
}

Flag Path::flags() const { return Object::flags() | Flag::IsPathLike; }

Path::iterator Path::end() { return ::omm::end<Path&>(*this); }
Path::iterator Path::begin() { return ::omm::begin<Path&>(*this); }

void Path::on_property_value_changed(Property* property)
{
  Object::on_property_value_changed(property);
  if (property == this->property(IS_CLOSED_PROPERTY_KEY)) {
    update();
  }
}

Geom::PathVector Path::paths() const
{
  return segments_to_path_vector(segments, is_closed());
}

template<typename PathRef>
Path::Iterator<PathRef>::Iterator(PathRef path, std::size_t segment, std::size_t point)
  : path(&path), segment(segment), point(point) {}

template<typename PathRef>
bool Path::Iterator<PathRef>::operator<(const Path::Iterator<PathRef>& other) const
{
  return iterator_to_tuple(*this) < iterator_to_tuple(other);
}

template<typename PathRef>
bool Path::Iterator<PathRef>::operator>(const Path::Iterator<PathRef>& other) const
{
  return iterator_to_tuple(*this) > iterator_to_tuple(other);
}

template<typename PathRef>
bool Path::Iterator<PathRef>::operator==(const Path::Iterator<PathRef>& other) const
{
  if (path != other.path) {
    return false;
  } if (is_end() && other.is_end()) {
    return true;
  } else {
    return segment == other.segment && point == other.point;
  }
}

template<typename PathRef>
bool Path::Iterator<PathRef>::operator!=(const Path::Iterator<PathRef>& other) const
{
  return !(*this == other);
}

template<typename PathRef>
bool Path::Iterator<PathRef>::is_end() const
{
  return segment >= path->segments.size();
}

template<typename PathRef>
typename Path::Iterator<PathRef>::reference Path::Iterator<PathRef>::operator*() const
{
  return path->segments[segment][point];
}

template<typename PathRef>
typename Path::Iterator<PathRef>::pointer Path::Iterator<PathRef>::operator->() const
{
  return &**this;
}

template<typename PathRef>
Path::Iterator<PathRef>& Path::Iterator<PathRef>::operator++()
{
  point += 1;
  if (path->segments[segment].size() == point) {
    point = 0;
    segment += 1;
  }
  return *this;
}

template struct Path::Iterator<Path&>;
template struct Path::Iterator<const Path&>;

}  // namespace omm
