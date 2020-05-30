#include "objects/abstractpath.h"
#include <algorithm>
#include "objects/empty.h"
#include <QObject>
#include "properties/floatproperty.h"
#include "objects/path.h"
#include "common.h"
#include "renderers/style.h"
#include "objects/tip.h"

namespace
{

omm::Point geom_to_point(const Geom::Point& point)
{
  return omm::Point({point.x(), point.y()});
}

double multclamp(double t, std::size_t n)
{
  if (n == 0) {
    return 0.0;
  } else {
    return std::clamp<double>(t * n, 0.0, n);
  }
}

}  // namespace

namespace omm
{

class Style;

AbstractPath::AbstractPath(Scene* scene)
  : Object(scene)
  , painter_path(*this)
  , geom_paths(*this)
{
}

void AbstractPath::draw_object(Painter &renderer, const Style& style,
                               Painter::Options options) const
{
  if (QPainter* painter = renderer.painter; painter != nullptr && is_active()) {
    renderer.set_style(style, *this, options);
    if (!is_closed()) {
      renderer.painter->setBrush(Qt::NoBrush);
    }
    painter->drawPath(painter_path());
    const auto marker_color = style.property(Style::PEN_COLOR_KEY)->value<Color>();
    const auto width = style.property(Style::PEN_WIDTH_KEY)->value<double>();
    style.start_marker.draw_marker(renderer, pos(0.0).rotated(.5 * M_PI), marker_color, width);
    style.end_marker.draw_marker(renderer, pos(1.0).rotated(3/2 * M_PI), marker_color, width);
  }
}

Flag AbstractPath::flags() const
{
  return Object::flags() | Flag::Convertible | Flag::IsPathLike;
}

std::unique_ptr<Object> AbstractPath::convert() const
{
  auto converted = std::make_unique<Path>(scene());
  copy_properties(*converted, CopiedProperties::Compatible | CopiedProperties::User);
  copy_tags(*converted);
  converted->set(geom_paths());
  converted->property(Path::IS_CLOSED_PROPERTY_KEY)->set(is_closed());
  converted->property(Path::INTERPOLATION_PROPERTY_KEY)->set(Path::InterpolationMode::Bezier);
  return std::unique_ptr<Object>(converted.release());
}


BoundingBox AbstractPath::bounding_box(const ObjectTransformation &transformation) const
{
  if (is_active()) {
    return BoundingBox((painter_path() * transformation.to_qtransform()).boundingRect());
  } else {
    return BoundingBox();
  }
}

void AbstractPath::update()
{
  painter_path.invalidate();
  geom_paths.invalidate();
  Object::update();
}

Point AbstractPath::pos(std::size_t segment, double t) const
{
  const Geom::PathVector paths = geom_paths()[segment];
  if (segment > paths.size()) {
    return geom_to_point(paths.initialPoint());
  } else {
    const Geom::Path path = paths.at(segment);
    if (const auto n = path.size_default(); n == 0) {
      return geom_to_point(path.initialPoint());
    } else {
      return geom_to_point(path.pointAt(multclamp(t, path.size_default())));
    }
  }
}

Point AbstractPath::pos(double t) const
{
  auto&& paths = geom_paths();
  if (const auto n = paths.curveCount(); n == 0) {
    return geom_to_point(paths.initialPoint());
  } else {
    return geom_to_point(paths.pointAt(multclamp(t, n)));
  }
}

Geom::Path AbstractPath::segment_to_path(const Segment &segment, bool is_closed) const
{
  const auto pts = [](const std::array<Vec2f, 4>& pts) {
    return ::transform<Geom::Point, std::vector>(pts, [](const auto& p) {
      return Geom::Point(p.x, p.y);
    });
  };

  std::vector<Geom::CubicBezier> bzs;
  const std::size_t n = segment.size();
  const std::size_t m = is_closed ? n : n - 1;
  for (std::size_t i = 0; i < m; ++i) {
    const std::size_t j = (i+1) % n;
    bzs.emplace_back(pts({ segment[i].position,
                           segment[i].right_position(),
                           segment[j].left_position(),
                           segment[j].position }));
  }
  return Geom::Path(bzs.begin(), bzs.end(), is_closed);
}

QPainterPath AbstractPath::CachedQPainterPathGetter::compute() const
{
  static const auto qpoint = [](const Geom::Point& point) { return QPointF{point[0], point[1]}; };
  const auto path_vector = m_self.paths();
  QPainterPath pp;
  for (const Geom::Path& path : path_vector) {
    pp.moveTo(qpoint(path.initialPoint()));
    for (const Geom::Curve& curve : path) {
      const auto& cbc = dynamic_cast<const Geom::CubicBezier&>(curve);
      pp.cubicTo(qpoint(cbc[1]), qpoint(cbc[2]), qpoint(cbc[3]));
    }
  }
  return pp;
}

Geom::PathVector AbstractPath::CachedGeomPathVectorGetter::compute() const
{
  return m_self.paths();
}

}  // namespace omm
