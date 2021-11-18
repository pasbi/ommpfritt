#pragma once

#include "cachedgetter.h"
#include "objects/object.h"
#include "disjointset.h"
#include <deque>
#include <list>
#include <type_traits>

namespace omm
{

class PathPoint;
class Scene;
class Segment;

class Path : public Object
{
public:
  explicit Path(Scene* scene);
  Path(const Path& other);
  Path(Path&&) = delete;
  Path& operator=(Path&&) = delete;
  Path& operator=(const Path&) = delete;
  ~Path();
  QString type() const override;

  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Path");
  static constexpr auto IS_CLOSED_PROPERTY_KEY = "closed";
  static constexpr auto SEGMENTS_POINTER = "segments";
  static constexpr auto INTERPOLATION_PROPERTY_KEY = "interpolation";

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;

  void update() override;
  bool is_closed() const override;
  void set(const Geom::PathVector& paths);


  void on_property_value_changed(Property* property) override;
  Geom::PathVector paths() const override;
  Flag flags() const override;

  std::size_t point_count() const;
  std::deque<Segment*> segments() const;
  Segment* find_segment(const PathPoint& point) const;
  Segment& add_segment(std::unique_ptr<Segment>&& segment);
  std::unique_ptr<Segment> remove_segment(const Segment& segment);
  std::deque<PathPoint*> points() const;
  std::deque<PathPoint*> selected_points() const;
  void deselect_all_points();

private:
  std::deque<std::unique_ptr<Segment>> m_segments;
  friend class JoinPointsCommand;
  friend class DisjoinPointsCommand;
};

}  // namespace omm
