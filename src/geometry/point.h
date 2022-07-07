#pragma once

#include "geometry/polarcoordinates.h"
#include "geometry/vec2.h"
#include <Qt>
#include <deque>
#include <map>
#include <set>

namespace omm
{

namespace serialization
{
class SerializerWorker;
class DeserializerWorker;
}  // namespace serialization

class Path;

class Point
{
public:
  enum class Direction { Forward, Backward };
  struct TangentKey
  {
    TangentKey(const Path* const path, Direction direction);
    TangentKey(const Direction direction);
    explicit TangentKey();
    const Path* path;
    Direction direction;

    void serialize(serialization::SerializerWorker& worker,
                   const std::map<const Path*, std::size_t>& path_indices) const;
    void deserialize(serialization::DeserializerWorker& worker, const std::vector<const Path*>& paths);

    QString to_string() const;
    bool operator<(const TangentKey& other) const noexcept;
    bool operator==(const TangentKey& other) const noexcept;
  };

  using TangentsMap = std::map<TangentKey, PolarCoordinates>;

  explicit Point(const Vec2f& position);
  explicit Point(const Vec2f& position, const PolarCoordinates& backward_tangent, const PolarCoordinates& forward_tangent);
  explicit Point(const Vec2f& position, const std::map<TangentKey, PolarCoordinates>& tangents);
  Point();
  [[nodiscard]] Vec2f position() const;
  void set_position(const Vec2f& position);

  void set_tangent_position(const TangentKey& key, const Vec2f& position);
  [[nodiscard]] Vec2f tangent_position(const TangentKey& key) const;
  [[nodiscard]] PolarCoordinates tangent(const TangentKey& key) const;
  void set_tangent(const TangentKey& key, const PolarCoordinates& vector);
  TangentsMap& tangents();
  const TangentsMap& tangents() const;
  void set_tangents(TangentsMap tangents);
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("Point", "Point");
  friend void swap(Point& a, Point& b);
  [[nodiscard]] double rotation() const;
  [[nodiscard]] bool has_nan() const;
  [[nodiscard]] bool has_inf() const;

  [[nodiscard]] Point rotated(double rad) const;
  [[nodiscard]] Point nibbed() const;

  static constexpr auto POSITION_POINTER = "position";
  static constexpr auto TANGENTS_POINTER = "tangents";

  void serialize(serialization::SerializerWorker& worker, const std::map<const Path*, std::size_t>& path_indices) const;
  void deserialize(serialization::DeserializerWorker& worker, const std::vector<const Path*> paths);

  bool operator==(const Point& point) const;
  bool operator!=(const Point& point) const;
  bool operator<(const Point& point) const;
  friend bool fuzzy_eq(const Point& a, const Point& b);

  [[nodiscard]] QString to_string() const;

  /**
   * @brief When a tangent is at `old_pos` and it is mirror-coupled with its sibling which moves
   *  from `old_other_pos` to `new_other_pos`, then this tangent moves to the return value of
   *  this function.
   * @param old_pos position of this tangent (relative to its Point)
   * @param old_other_pos old position of the other tangent (relative to its Point)
   * @param new_other_pos new position of the other tangent (relative to its Point)
   * @return [description]
   */
  static PolarCoordinates mirror_tangent(const PolarCoordinates& old_pos,
                                         const PolarCoordinates& old_other_pos,
                                         const PolarCoordinates& new_other_pos);

private:
  Vec2f m_position;
  TangentsMap m_tangents;
};

constexpr PolarCoordinates to_polar(Vec2f cartesian);
constexpr Vec2f to_cartesian(const PolarCoordinates& polar);

}  // namespace omm
