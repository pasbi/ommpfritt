#pragma once

#include <QDebug>
#include "objects/abstractpath.h"
#include "geometry/point.h"
#include <list>
#include "cachedgetter.h"
#include <type_traits>

namespace omm
{

class Scene;

class Path : public AbstractPath
{
public:
  explicit Path(Scene* scene);
  void draw_object(Painter& renderer, const Style& style, Painter::Options options) const override;
  BoundingBox bounding_box(const ObjectTransformation& transformation) const override;
  QString type() const override;

  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Path");
  static constexpr auto IS_CLOSED_PROPERTY_KEY = "closed";
  static constexpr auto SUBPATH_POINTER = "paths";
  static constexpr auto INTERPOLATION_PROPERTY_KEY = "interpolation";

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;

  enum class InterpolationMode { Linear, Smooth, Bezier };
  Flag flags() const override;
  void update() override;
  bool is_closed() const override;
  void set(const Geom::PathVector& paths);

  std::vector<Segment> segments;

  template<typename PathRef>
  struct Iterator
  {
    using value_type = Point;
    static_assert(std::is_reference_v<PathRef>);
    static constexpr bool Const = std::is_const_v<std::remove_reference_t<PathRef>>;
    using reference = std::conditional_t<Const, const value_type&, value_type&>;
    using pointer = std::conditional_t<Const, const value_type*, value_type*>;
    using difference_type = int;
    using iterator_category = std::forward_iterator_tag;

    Iterator(PathRef path, std::size_t segment, std::size_t point);

    std::add_pointer_t<std::remove_reference_t<PathRef>> path;
    std::size_t segment;
    std::size_t point;

    bool operator<(const Iterator& other) const;
    bool operator>(const Iterator& other) const;
    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;

    bool is_end() const;
    reference operator*() const;
    pointer operator->() const;

    Iterator& operator++();
  };

  using iterator = Iterator<Path&>;

  iterator begin();
  iterator end();
  void on_property_value_changed(Property* property) override;
  Geom::PathVector paths() const override;

};

template<typename PathRef> auto begin(PathRef p)
{
  return Path::Iterator<PathRef>{p, 0, 0};
}

template<typename PathRef> auto end(PathRef p)
{
  return Path::Iterator<PathRef>{p, p.segments.size(), 0};
}

}  // namespace omm
