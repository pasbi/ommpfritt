#pragma once

#include "objects/object.h"
#include <2geom/pathvector.h>
#include "cachedgetter.h"
#include <QPainterPath>
#include <Qt>

namespace omm
{

class Scene;

class AbstractPath : public Object
{
public:
  explicit AbstractPath(Scene* scene);
  void draw_object(Painter& renderer, const Style& style, Painter::Options options) const override;
  BoundingBox bounding_box(const ObjectTransformation& transformation) const override;
  std::unique_ptr<Object> convert() const override;

  Flag flags() const override;
  void update() override;
  virtual Geom::PathVector paths() const = 0;
  Point pos(std::size_t segment, double t) const override;
  Point pos(double t) const override;

  struct CachedQPainterPathGetter : CachedGetter<QPainterPath, AbstractPath>
  {
    using CachedGetter::CachedGetter;
  private:
    QPainterPath compute() const override;
  } painter_path;

  struct CachedGeomPathVectorGetter : CachedGetter<Geom::PathVector, AbstractPath>
  {
    using CachedGetter::CachedGetter;
  private:
    Geom::PathVector compute() const override;
  } geom_paths;

  friend struct CachedQPainterPathGetter;

  using Segment = std::vector<Point>;

  Geom::Path segment_to_path(const Segment& segment, bool is_closed) const;

  template<typename Segments=std::vector<Segment>>
  Geom::PathVector segments_to_path_vector(const Segments& segments, bool is_closed) const
  {
    Geom::PathVector paths;
    for (auto&& segment : segments) {
      paths.push_back(segment_to_path(segment, is_closed));
    }
    return paths;
  }
};

}  // namespace omm
