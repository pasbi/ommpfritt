#include "objects/path.h"

#include <QObject>
#include "commands/modifypointscommand.h"
#include "properties/boolproperty.h"
#include "properties/optionproperty.h"
#include "scene/scene.h"
#include "geometry/cubics.h"
#include "common.h"
#include "renderers/style.h"

namespace omm
{

class Style;

Path::Path(Scene* scene)
  : Object(scene)
  , painter_path(*this)
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
  renderer.set_style(style, *this, options);
  renderer.painter->drawPath(painter_path());
  const auto marker_color = style.property(Style::PEN_COLOR_KEY)->value<Color>();
  const auto width = style.property(Style::PEN_WIDTH_KEY)->value<double>();
  style.start_marker.draw_marker(renderer, evaluate(0.0).rotated(0.5 * M_PI), marker_color, width);
  style.end_marker.draw_marker(renderer, evaluate(1.0).rotated(1.5 * M_PI), marker_color, width);
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
  serializer.start_array(m_segments.size(), subpath_ptr);
  for (std::size_t i = 0; i < m_segments.size(); ++i) {
    const auto pts_ptr = make_pointer(subpath_ptr, i);
    serializer.start_array(m_segments.size(), subpath_ptr);
    std::size_t j = 0;
    for (auto&& point : m_segments[i]) {
      point.serialize(serializer, make_pointer(pts_ptr, j));
      j += 1;
    }
    serializer.end_array();
  }
  serializer.end_array();
}

void Path::deserialize(AbstractDeserializer& deserializer, const Pointer& root)
{
  Object::deserialize(deserializer, root);

  const auto subpath_ptr = make_pointer(root, SUBPATH_POINTER);
  const std::size_t n_paths = deserializer.array_size(subpath_ptr);
  m_segments.clear();
  m_segments.reserve(n_paths);
  for (size_t i = 0; i < n_paths; ++i) {
    const auto pts_ptr = make_pointer(subpath_ptr, i);
    const std::size_t n_points = deserializer.array_size(pts_ptr);
    m_segments.push_back({});
    for (size_t j = 0; j < n_points; ++j) {
      Point p;
      p.deserialize(deserializer, make_pointer(pts_ptr, j));
      m_segments[i].push_back(p);
    }
  }
  update();
}

void Path::update()
{
  painter_path.invalidate();
  Object::update();
}

void Path::insert(const const_iterator& pos, const Segment& points)
{
  assert(!m_segments.empty());
  if (pos.segment_iterator() == m_segments.end()) {
    m_segments.push_back(Segment{});
    m_segments.back().insert(m_segments.back().begin(), points.begin(), points.end());
  } else {
    const auto ncpos = remove_const(pos);
    Segment& segment = *ncpos.segment_iterator();
    segment.insert(ncpos.point_iterator(), points.begin(), points.end());
  }
}

Flag Path::flags() const { return Object::flags() | Flag::IsPathLike; }

QPainterPath Path::CachedQPainterPathGetter::compute() const
{
  static const auto p = [](const Vec2f& v) { return QPointF{v.x, v.y}; };
  QPainterPath path;
  for (auto&& points : m_self.m_segments) {
    if (!points.empty()) {
      path.moveTo(p(points.front().position));
    }
    auto previous = points.begin();
    auto current = previous;
    ++current;
    while (current != points.end()) {
      path.cubicTo(p(previous->right_position()),
                   p(current->left_position()),
                   p(current->position));
      previous = current;
      ++current;
    }
    if (m_self.is_closed()) {
      path.cubicTo(p(points.back().right_position()),
                   p(points.front().left_position()),
                   p(points.front().position));
    }
  }
  return path;
}

Path::const_iterator begin(const Path& path) { return path.begin(); }
Path::const_iterator end(const Path& path) { return path.end(); }
Path::iterator begin(Path& path) { return path.begin(); }
Path::iterator end(Path& path) { return path.end(); }

}  // namespace omm
