#include "geometry/point.h"
#include "logging.h"
#include "serializers/deserializerworker.h"
#include "serializers/serializerworker.h"
#include <cmath>

namespace
{

constexpr auto KEY_KEY = "key";
constexpr auto VAL_KEY = "val";
constexpr auto PATH_KEY = "path";
constexpr auto DIRECTION_KEY = "direction";
constexpr auto PATH_IS_SET_KEY = "path is set";

}  // namespace

namespace omm
{

Point::Point(const Vec2f& position) : Point(position, {})
{
}

Point::Point(const Vec2f& position, const PolarCoordinates& backward_tangent, const PolarCoordinates& forward_tangent)
    : Point(position, {{Direction::Backward, backward_tangent}, {Direction::Forward, forward_tangent}})
{
}

Point::Point(const Vec2f& position, const std::map<TangentKey, PolarCoordinates>& tangents)
    : m_position(position), m_tangents(tangents)
{
}

Point::Point() : Point(Vec2f::o())
{
}

Vec2f Point::position() const
{
  return m_position;
}

void Point::set_position(const Vec2f& position)
{
  m_position = position;
}

Vec2f Point::tangent_position(const TangentKey& key) const
{
  return m_position + tangent(key).to_cartesian();
}

void Point::set_tangent_position(const TangentKey& key, const Vec2f& position)
{
  set_tangent(key, PolarCoordinates(position - m_position));
}

PolarCoordinates Point::tangent(const TangentKey& key) const
{
  return m_tangents.at(key);
}

void Point::set_tangent(const TangentKey& key, const PolarCoordinates& vector)
{
  m_tangents[key] = vector;
}

Point::TangentsMap& Point::tangents()
{
  return m_tangents;
}

const Point::TangentsMap& Point::tangents() const
{
  return m_tangents;
}

/**
 * @brief replace_tangents_key replaces the key `{old_path, *}` with `{new_path, *}`
 *  or adds null-tangents at `{new_path, *}` (`*` stands for both directions).
 *  `old_path` and `new_path` are passed as key-value pairs via `paths_map`.
 */
void Point::replace_tangents_key(const std::map<const Path*, Path*>& paths_map)
{
  for (const auto& [old_path, new_path] : paths_map) {
    for (const auto& direction : {omm::Direction::Backward, omm::Direction::Forward}) {
      const auto node = m_tangents.extract({old_path, direction});
      m_tangents.try_emplace({new_path, direction}, node.empty() ? omm::PolarCoordinates() : node.mapped());
    }
  }
}

void swap(Point& a, Point& b)
{
  swap(a.m_position, b.m_position);
  swap(a.m_tangents, b.m_tangents);
}

bool Point::has_nan() const
{
  return m_position.has_nan() || std::any_of(m_tangents.begin(), m_tangents.end(), [](const auto& p) {
    return p.second.has_nan();
  });
}

bool Point::has_inf() const
{
  return m_position.has_inf() || std::any_of(m_tangents.begin(), m_tangents.end(), [](const auto& p) {
    return p.second.has_inf();
  });
}

double Point::rotation() const
{
  if (const auto it = m_tangents.begin(); it != m_tangents.end()) {
    return it->second.argument;
  } else {
    return 0.0;
  }
}

Point Point::nibbed() const
{
  auto copy = *this;
  for (auto& [key, tangent] : copy.m_tangents) {
    tangent.magnitude = 0.0;
  }
  return copy;
}

void Point::serialize(serialization::SerializerWorker& worker,
                      const std::map<const Path*, std::size_t>& path_indices) const
{
  worker.sub(POSITION_POINTER)->set_value(m_position);
  worker.sub(TANGENTS_POINTER)->set_value(m_tangents, [&path_indices](const auto& pair, auto& worker) {
    pair.first.serialize(*worker.sub(KEY_KEY), path_indices);
    worker.sub(VAL_KEY)->set_value(pair.second);
  });
}

void Point::deserialize(serialization::DeserializerWorker& worker, const std::vector<const Path*> paths)
{
  m_position = worker.sub(POSITION_POINTER)->get<Vec2f>();
  worker.sub(TANGENTS_POINTER)->get_items([&paths, this](auto& worker) {
    const auto value = worker.sub(VAL_KEY)->template get<PolarCoordinates>();
    TangentKey key;
    key.deserialize(*worker.sub(KEY_KEY), paths);
    m_tangents.emplace(key, value);
  });
}

QString Point::to_string() const
{
  static constexpr bool verbose = false;
  if constexpr (verbose) {
    const auto tangents = util::transform<QList>(m_tangents, [](const auto& pair) {
      const auto& [key, tangent] = pair;
      return QString("%1: %2").arg(key.to_string(), tangent.to_string());
    });
    return QString{"Point[%1;\n  %2]"}.arg(m_position.to_string(), QStringList(tangents).join("\n  "));
  } else {
    return QString{"[%1]"}.arg(m_position.to_string());
  }
}

QRectF Point::bounding_box() const
{
  const auto get_tangent_position = [this](const auto& t) { return tangent_position(t.first); };
  auto ps = util::transform<std::list>(m_tangents, get_tangent_position);
  ps.emplace_back(position());

  static constexpr auto cmp_x = [](const auto& a, const auto& b) { return a.x < b.x; };
  static constexpr auto cmp_y = [](const auto& a, const auto& b) { return a.y < b.y; };

  const QPointF min(std::min_element(ps.begin(), ps.end(), cmp_x)->x,
                    std::min_element(ps.begin(), ps.end(), cmp_y)->y);
  const QPointF max(std::max_element(ps.begin(), ps.end(), cmp_x)->x,
                    std::max_element(ps.begin(), ps.end(), cmp_y)->y);
  return {min, max};
}

QRectF Point::bounding_box(const std::list<Point>& points)
{
  static constexpr auto get_bb = [](const auto& p) { return p.bounding_box(); };
  const auto bbs = util::transform(points, get_bb);
  const auto tls = util::transform(bbs, &QRectF::topLeft);
  const auto brs = util::transform(bbs, &QRectF::bottomRight);
  static constexpr auto cmp_x = [](const auto& a, const auto& b) { return a.x() < b.x(); };
  static constexpr auto cmp_y = [](const auto& a, const auto& b) { return a.y() < b.y(); };

  const QPointF tl(std::min_element(tls.begin(), tls.end(), cmp_x)->x(),
                   std::min_element(tls.begin(), tls.end(), cmp_y)->y());
  const QPointF br(std::max_element(brs.begin(), brs.end(), cmp_x)->x(),
                   std::max_element(brs.begin(), brs.end(), cmp_y)->y());
  return {tl, br};
}

bool Point::operator==(const Point& point) const
{
  return m_position == point.m_position && m_tangents == point.m_tangents;
}

bool Point::operator!=(const Point& point) const
{
  return !(*this == point);
}

bool Point::operator<(const Point& point) const
{
  if (m_position == point.m_position) {
    return m_tangents < point.m_tangents;
  } else {
    return m_position < point.m_position;
  }
}

bool fuzzy_eq(const Point& a, const Point& b)
{
  if (!fuzzy_eq(a.position(), b.position())) {
    return false;
  }

  const auto keys = get_keys(a.m_tangents);
  if (keys != get_keys(b.m_tangents)) {
    return false;
  }

  return std::all_of(keys.begin(), keys.end(), [&a, &b](const auto& key) {
    return fuzzy_eq(a.tangent_position(key), b.tangent_position(key));
  });
}

bool omm::Point::TangentKey::operator<(const omm::Point::TangentKey& other) const noexcept
{
  static constexpr auto to_tuple = [](const auto& self) {
    return std::tuple(self.path, self.direction);
  };
  return to_tuple(*this) < to_tuple(other);
}

bool Point::TangentKey::operator==(const TangentKey& other) const noexcept
{
  return path == other.path && direction == other.direction;
}

QString omm::Point::TangentKey::to_string() const
{
  return QString::asprintf("%p-%s", static_cast<const void*>(path), direction == Direction::Forward ? "fwd" : "bwd");
}

Point::TangentKey::TangentKey(const Path* const path, const Direction direction) : path(path), direction(direction)
{
}

Point::TangentKey::TangentKey(const Direction direction) : TangentKey(nullptr, direction)
{
}

Point::TangentKey::TangentKey()
{
}

void omm::Point::TangentKey::serialize(serialization::SerializerWorker& worker,
                                       const std::map<const Path*, std::size_t>& path_indices) const
{
  worker.sub(PATH_IS_SET_KEY)->set_value(path != nullptr);
  if (path != nullptr) {
    worker.sub(PATH_KEY)->set_value(path_indices.at(path));
  }
  worker.sub(DIRECTION_KEY)->set_value(static_cast<std::underlying_type_t<Direction>>(direction));
}

void omm::Point::TangentKey::deserialize(serialization::DeserializerWorker& worker,
                                         const std::vector<const Path*>& paths)
{
  if (worker.sub(PATH_IS_SET_KEY)->get_bool()) {
    this->path = paths.at(worker.sub(PATH_KEY)->get<std::size_t>());
  } else {
    this->path = nullptr;
  }
  this->direction = static_cast<Direction>(worker.sub(DIRECTION_KEY)->get<std::underlying_type_t<Direction>>());
}

PolarCoordinates Point::mirror_tangent(const PolarCoordinates& old_pos,
                                       const PolarCoordinates& old_other_pos,
                                       const PolarCoordinates& new_other_pos)
{
  PolarCoordinates new_pos;
  static constexpr double mag_eps = 0.00001;
  new_pos.argument = old_pos.argument + new_other_pos.argument - old_other_pos.argument;
  if (old_other_pos.magnitude > mag_eps) {
    new_pos.magnitude = old_pos.magnitude * new_other_pos.magnitude / old_other_pos.magnitude;
  }
  return new_pos;
}

}  // namespace omm
