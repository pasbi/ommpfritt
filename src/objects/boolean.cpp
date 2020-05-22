#include "objects/boolean.h"
#include "properties/optionproperty.h"
#include <2geom/2geom.h>
#include <2geom/utils.h>
#include <2geom/intersection-graph.h>
#include <2geom/pathvector.h>

namespace
{

std::vector<omm::Point> get_points(const Geom::Path& path)
{
  static const auto to_vec2f = [](const Geom::Point& pt) { return omm::Vec2f{pt.x(), pt.y()}; };
  static constexpr auto eps2 = 0.00001;
  std::list<omm::Point> points;
  const auto make_tangent = [&points](const Geom::Point& pos) {
    return omm::PolarCoordinates(to_vec2f(pos) - points.back().position);
  };
  for (const auto& curve : path) {
    if (const auto* bz = dynamic_cast<const Geom::CubicBezier*>(&curve); bz != nullptr) {
      if (points.empty()) {
        points.emplace_back(to_vec2f(bz->controlPoint(0)));
      } else {
        if ((points.back().position - to_vec2f(bz->controlPoint(0))).euclidean_norm2() > eps2) {
          LWARNING << "non-continuous spline.";
        }
        points.back().right_tangent = make_tangent(bz->controlPoint(1));
      }

      points.emplace_back(to_vec2f(bz->controlPoint(3)));
      points.back().left_tangent = make_tangent(bz->controlPoint(2));
    }
  }

  return std::vector(points.begin(), points.end());
}

using F = std::function<Geom::PathVector(Geom::PathIntersectionGraph&)>;
static const std::vector<std::pair<QString, F>> dispatcher = {
  {QObject::tr("Union"),              [](auto& pig) { return pig.getUnion(); }},
  {QObject::tr("Intersection"),       [](auto& pig) { return pig.getIntersection(); }},
  {QObject::tr("Exclusive Or"),       [](auto& pig) { return pig.getXOR(); }},
  {QObject::tr("Difference"),         [](auto& pig) { return pig.getAminusB(); }},
  {QObject::tr("Inverse Difference"), [](auto& pig) { return pig.getBminusA(); }},
};

}  // namespace

namespace omm
{

Boolean::Boolean(Scene* scene)
  : AbstractProceduralPath(scene)
{
  create_property<OptionProperty>(MODE_PROPERTY_KEY)
  .set_options(::transform<QString>(dispatcher, [](auto&& p) { return p.first; }))
  .set_label(QObject::tr("mode"))
  .set_category(QObject::tr("Boolean"));
  polish();
}

Boolean::Boolean(const Boolean& other)
  : AbstractProceduralPath(other)
{
  polish();
}

QString Boolean::type() const
{
  return TYPE;
}

Flag Boolean::flags() const
{
  return Flag::IsPathLike | Flag::Convertible;
}

void Boolean::update()
{
  m_draw_children = !is_active();
  AbstractProceduralPath::update();
}

void Boolean::on_property_value_changed(Property* property)
{
  if (property == this->property(MODE_PROPERTY_KEY)) {
    update();
  }
  AbstractProceduralPath::on_property_value_changed(property);
}

void Boolean::polish()
{
  listen_to_children_changes();
}

std::vector<Point> Boolean::points() const
{
  const auto children = tree_children();
  if (is_active() && children.size() == 2) {
    Geom::PathIntersectionGraph pig(Geom::PathVector(children[0]->path()),
                                    Geom::PathVector(children[1]->path()));
    if (pig.valid()) {
      const auto i = property(MODE_PROPERTY_KEY)->value<std::size_t>();
      const auto paths = dispatcher[i].second(pig);
      if (paths.size() == 0) {
        return {};
      } else {
        auto points = get_points(paths.front());
        return points;
      }
    } else {
      return {};
    }
  }
  return {};
}

bool Boolean::is_closed() const
{
  return true;
}

}  // namespace omm
