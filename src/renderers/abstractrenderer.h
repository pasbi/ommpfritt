#pragma once

#include <armadillo>
#include <string>

namespace omm
{

class Style;
class Scene;

class AbstractRenderer
{
public:
  class Region
  {
  public:
    Region(const arma::vec2& point);
    Region(const arma::vec2& top_left, const arma::vec2& top_right);

    Region merge(const Region& other) const;
    Region intersect(const Region& other) const;
    bool contains(const Region& other) const;
    bool contains(const arma::vec2& point) const;
    bool is_empty() const;

    arma::vec2 top_left() const;
    arma::vec2 top_right() const;
    arma::vec2 bottom_left() const;
    arma::vec2 bottom_right() const;

  private:
    arma::vec2 m_top_left;
    arma::vec2 m_bottom_right;
  };

  explicit AbstractRenderer(const Region& region);
  void render(const Scene& scene);
  const Region& region() const;

protected:
  class Point
  {
  public:
    arma::vec2 position;
    arma::vec2 left_tangent;
    arma::vec2 right_tangent;
  };


  virtual void start_group(const std::string& name) {}
  virtual void end_group() {}
  virtual void draw_spline(const std::vector<Point>& points, const Style& style) = 0;

private:
  const Region m_region;
};

}  // namespace omm
