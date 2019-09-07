#pragma once

#include "objects/object.h"
#include "geometry/point.h"
#include <list>
#include "geometry/cubics.h"

namespace omm
{

class Scene;

class Path : public Object
{
public:
  explicit Path(Scene* scene);
  void draw_object(Painter& renderer, const Style& style) const override;
  BoundingBox bounding_box(const ObjectTransformation& transformation) const override;
  std::string type() const override;
  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "Path");
  std::unique_ptr<Object> clone() const override;
  std::vector<Point> points() const override;
  std::vector<Point*> points_ref();
  Point& point(std::size_t i);
  const Point& point(std::size_t i) const;
  Cubics cubics() const;
  void set_points(const std::vector<Point>& points);
  static constexpr auto IS_CLOSED_PROPERTY_KEY = "closed";
  static constexpr auto POINTS_POINTER = "points";
  static constexpr auto INTERPOLATION_PROPERTY_KEY = "interpolation";

  void serialize(AbstractSerializer& serializer, const Pointer& root) const override;
  void deserialize(AbstractDeserializer& deserializer, const Pointer& root) override;
  bool tangents_modifiable() const;

  enum class InterpolationMode { Linear, Smooth, Bezier };
  void deselect_all_points();
  std::vector<std::size_t> selected_points() const;

  std::map<Point*, Point>
  modified_points(const bool constrain_to_selection, InterpolationMode mode);
  PathUniquePtr outline(const double t) const override;

  struct PointSequence
  {
    explicit PointSequence(int position);
    explicit PointSequence(int position, const std::initializer_list<Point>& points);
    explicit PointSequence(int position, const std::list<Point>& points);
    explicit PointSequence(int position, const std::vector<Point>& points);
    std::size_t position = 0;
    std::list<Point> sequence;
  };

  std::vector<PointSequence> remove_points(std::vector<std::size_t> points);
  std::vector<std::size_t> add_points(std::vector<PointSequence> sequences);
  std::vector<PointSequence> get_point_sequences(const std::vector<double> &ts) const;
  void update_tangents();

  bool is_closed() const override;
  Flag flags() const override;

  void set_global_axis_transformation( const ObjectTransformation& global_transformation,
                                       Space space ) override;

  std::vector<double> cut(const Vec2f& c_start, const Vec2f& c_end);
  Point smoothed(const std::size_t& i) const;

  Point evaluate(const double t) const override;
  double path_length() const override;
  bool contains(const Vec2f &pos) const override;
  void update() override;

protected:
  void on_property_value_changed(Property *property) override;

private:
  std::vector<Point> m_points;
  /**
   * @brief this function does not notifiy the active tool.
   *  use the overload add_points(const std::vector<PointSequence>&);
   */
  std::vector<std::size_t> add_points(const PointSequence& sequence);
  QPainterPath m_painter_path;
};

}  // namespace omm
