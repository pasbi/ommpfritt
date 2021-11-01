#pragma once

#include "cachedgetter.h"
#include "objects/object.h"
#include "objects/pathiterator.h"
#include <list>
#include <type_traits>

namespace omm
{
class Scene;

class Path : public Object
{
public:
  explicit Path(Scene* scene);
  QString type() const override;

  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Path");
  static constexpr auto IS_CLOSED_PROPERTY_KEY = "closed";
  static constexpr auto SUBPATH_POINTER = "paths";
  static constexpr auto INTERPOLATION_PROPERTY_KEY = "interpolation";

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;

  void update() override;
  bool is_closed() const override;
  void set(const Geom::PathVector& paths);

  std::vector<Segment> segments;
  std::size_t count() const;

  PathIterator begin();
  PathIterator end();
  void on_property_value_changed(Property* property) override;
  Geom::PathVector paths() const override;
  static Point smoothen_point(const Segment& segment, bool is_closed, std::size_t i);
  Flag flags() const override;
};

template<typename PathRef> auto begin(PathRef p)
{
  return PathIteratorBase<PathRef>{p, 0, 0};
}

template<typename PathRef> auto end(PathRef p)
{
  return PathIteratorBase<PathRef>{p, p.segments.size(), 0};
}

}  // namespace omm
