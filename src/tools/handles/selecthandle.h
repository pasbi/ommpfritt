#pragma once

#include <armadillo>
#include "tools/handles/handle.h"
#include "tools/handles/particlehandle.h"

namespace omm
{

class Scene;
class Path;

template<typename PositionVariant> class SelectTool;
class PointPositions;
class ObjectPositions;

class ObjectSelectHandle : public Handle
{
public:
  explicit ObjectSelectHandle(SelectTool<ObjectPositions>& tool, Scene& scene, Object& object);
  bool contains(const arma::vec2& point) const override;
  void draw(omm::AbstractRenderer& renderer) const override;
  bool mouse_press( const arma::vec2& pos,
                    Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers ) override;
  bool mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover) override;

protected:
  ObjectTransformation transformation() const override;

private:
  SelectTool<ObjectPositions>& m_tool;
  Scene& m_scene;
  Object& m_object;
};

class PointSelectHandle : public Handle
{
public:
  enum class Tangent { Left, Right };
  enum class TangentMode { Mirror, Individual };
  explicit PointSelectHandle(SelectTool<PointPositions>& tool, Path& path, Point& point);
  bool contains(const arma::vec2& point) const override;
  void draw(omm::AbstractRenderer& renderer) const override;
  bool mouse_press( const arma::vec2& pos,
                    Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers ) override;
  bool mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover) override;
  void mouse_release(const arma::vec2& pos) override;

  template<Tangent tangent>
  void transform_tangent(const arma::vec2& delta);

protected:
  ObjectTransformation transformation() const override;

private:
  SelectTool<PointPositions>& m_tool;
  Path& m_path;
  Point& m_point;
  const std::unique_ptr<Style> m_tangent_style;
  std::unique_ptr<ParticleHandle> m_left_tangent_handle;
  std::unique_ptr<ParticleHandle> m_right_tangent_handle;
  bool tangents_active() const;

  template<Tangent tangent>
  void transform_tangent(const arma::vec2& delta, TangentMode mode);
};



}  // namespace omm
