#pragma once

#include "objects/object.h"
#include "geometry/point.h"
#include <list>
#include "geometry/cubics.h"
#include "cachedgetter.h"

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

  using Segment = std::list<Point>;

private:
public:
  std::vector<Segment> m_segments = { Segment{} };

  struct CachedQPainterPathGetter : CachedGetter<QPainterPath, Path>
  {
    using CachedGetter::CachedGetter;
  private:
    QPainterPath compute() const override;
  } painter_path;
  friend struct CachedQPainterPathGetter;

  template<bool Const>
  struct Iterator
  {
  public:
    using difference_type = int;
    using value_type = Path;
    using pointer = std::conditional_t<Const, const Path*, Path*>;
    using reference = std::conditional_t<Const, const Path& , Path&>;
    using iterator_category = std::bidirectional_iterator_tag;

    using point_iterator_type = std::conditional_t<Const, Segment::const_iterator,
                                                          Segment::iterator>;
    using segment_iterator_type = std::conditional_t<Const, std::vector<Segment>::const_iterator,
                                                            std::vector<Segment>::iterator>;
    explicit Iterator(reference path, const segment_iterator_type& segment_iterator,
                                      const point_iterator_type& point_iterator)
      : m_path(path)
      , m_segment_iterator(segment_iterator)
      , m_point_iterator(point_iterator)
    {
    }

    static auto begin(reference path)
    {
      assert(!path.m_segments.empty());
      return Iterator{path, path.m_segments.begin(), path.m_segments.front().begin()};
    }

    static auto end(reference path)
    {
      assert(!path.m_segments.empty());
      return Iterator(path, path.m_segments.end() - 1, path.m_segments.back().end());
    }

    bool operator==(const Iterator& other) const
    {
      LINFO << (this->m_point_iterator == other.m_point_iterator);
      return &this->m_path == &other.m_path
          && this->m_segment_iterator == other.m_segment_iterator
          && this->m_point_iterator == other.m_point_iterator;
    }

    bool operator!=(const Iterator& other) const
    {
      return !(*this == other);
    }

    Iterator& operator--()
    {
      if (m_point_iterator == m_segment_iterator->begin()) {
        --m_segment_iterator;
        m_point_iterator == m_segment_iterator->end();
      } else {
        --m_point_iterator;
      }
      return *this;
    }

    Iterator& operator++()
    {
      ++m_point_iterator;
      if (m_point_iterator == m_segment_iterator->end()) {
        ++m_segment_iterator;
        if (m_segment_iterator == m_path.m_segments.end()) {
          --m_segment_iterator;
          m_point_iterator = m_segment_iterator->end();
        } else {
          m_point_iterator = m_segment_iterator->begin();
        }
      }
      return *this;
    }

    decltype(auto) operator*() const
    {
      return *m_point_iterator;
    }

    operator Iterator<true>() const
    {
      return Iterator<true>{m_path, m_segment_iterator, m_point_iterator};
    }

    point_iterator_type point_iterator() const
    {
      return m_point_iterator;
    }

    segment_iterator_type segment_iterator() const
    {
      return m_segment_iterator;
    }

    bool is_segment_begin() const
    {
      return m_point_iterator == m_segment_iterator->begin();
    }

    bool is_segment_ultimo() const
    {
      auto end = m_segment_iterator->end();
      --end;
      return m_point_iterator == end;
    }

  private:
    reference m_path;
    segment_iterator_type m_segment_iterator;
    point_iterator_type m_point_iterator;
    friend class Path;
  };

  template<bool> friend struct Iterator;

public:
  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;
  decltype(auto) begin() { return iterator::begin(*this); }
  decltype(auto) end() { return iterator::end(*this); }
  decltype(auto) begin() const { return const_iterator::begin(*this); }
  decltype(auto) end() const { return const_iterator::end(*this); }
  iterator remove_const(const const_iterator& it)
  {
    assert(&it.m_path == this);
    const auto seg_it = m_segments.erase(it.m_segment_iterator, it.m_segment_iterator);
    const auto pt_it = seg_it->erase(it.m_point_iterator, it.m_point_iterator);
    return iterator{*this, seg_it, pt_it};
  }

  void insert(const const_iterator& pos, const Segment& points);
  const auto& segments() const { return m_segments; }
};

Path::const_iterator begin(const Path& path);
Path::const_iterator end(const Path& path);
Path::iterator begin(Path& path);
Path::iterator end(Path& path);

}  // namespace omm
