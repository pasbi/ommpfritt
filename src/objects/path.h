#pragma once

#include <QDebug>
#include "objects/object.h"
#include "geometry/point.h"
#include <list>
#include "geometry/cubics.h"
#include "cachedgetter.h"
#include <type_traits>

namespace omm
{

class Scene;

class Path : public Object
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

  using Segment = std::vector<Point>;

  template<typename R, typename Path, typename F> static auto foreach_point(Path path, F&& f)
  {
    static constexpr auto use_return = !std::is_same_v<R, void>;
    std::conditional_t<use_return, std::list<R>, int> rs;
    for (auto&& segment : path.segments) {
      for (auto&& p : segment) {
        if constexpr (use_return) {
          rs.push_back(f(p));
        }
      }
    }
    if constexpr (use_return) {
      return rs;
    }
  }

  std::vector<Segment> segments;

  template<typename PathRef>
  struct Iterator
  {
    using value_type = Point;
    static constexpr bool Const = std::is_const_v<PathRef>;
    using reference = std::conditional_t<Const, const value_type&, value_type&>;
    using pointer = std::conditional_t<Const, const value_type&, value_type&>;
    using difference_type = int;
    using iterator_category = std::forward_iterator_tag;

    Iterator(PathRef path, std::size_t segment, std::size_t point)
      : path(&path), segment(segment), point(point) {}

    std::add_pointer_t<std::remove_const_t<PathRef>> path;
    std::size_t segment;
    std::size_t point;

    bool operator<(const Iterator& other) const
    {
      return segment == other.segment ? point < other.point : segment < other.segment;
    }

    bool operator>(const Iterator& other) const
    {
      return segment == other.segment ? point > other.point : segment > other.segment;
    }

    bool operator==(const Iterator& other) const
    {
      if (is_end() && other.is_end()) {
        return true;
      } else {
        return segment == other.segment && point == other.point;
      }
    }

    bool is_end() const { return segment >= path->segments.size(); }
    decltype(auto) operator*() const { return path->segments[segment][point]; }
    decltype(auto) operator->() const { return &**this; }
    bool operator!=(const Iterator& other) const { return !(*this == other); }

    Iterator& operator++()
    {
      point += 1;
      if (path->segments[segment].size() == point) {
        point = 0;
        segment += 1;
      }
      return *this;
    }
  };

  using const_iterator = Iterator<const Path&>;
  using iterator = Iterator<Path&>;

  const_iterator begin() const;
  const_iterator end() const;
  iterator begin();
  iterator end();
  iterator remove_const(const const_iterator& it);
  std::size_t count() const;
  bool is_closed() const override;
  void on_property_value_changed(Property* property) override;

  struct CachedQPainterPathGetter : CachedGetter<QPainterPath, Path>
  {
    using CachedGetter::CachedGetter;
  private:
    QPainterPath compute() const override;
  } painter_path;
  friend struct CachedQPainterPathGetter;
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
