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

class AbstractSelectHandle : public Handle
{
public:
  void mouse_release( const arma::vec2& pos, const QMouseEvent& event) override;
  bool mouse_move(const arma::vec2& delta, const arma::vec2& pos, const QMouseEvent& e) override;
protected:
  virtual void set_selected(bool selected) = 0;
  virtual void clear() = 0;
  virtual bool is_selected() const = 0;
  void report_move_action();

private:
  bool m_move_was_performed = false;

  static constexpr auto extend_selection_modifier = Qt::ShiftModifier;
};

class ObjectSelectHandle : public AbstractSelectHandle
{
public:
  explicit ObjectSelectHandle(SelectTool<ObjectPositions>& tool, Scene& scene, Object& object);
  bool contains(const arma::vec2& point) const override;
  void draw(omm::AbstractRenderer& renderer) const override;
  bool mouse_press( const arma::vec2& pos, const QMouseEvent& event) override;
  void mouse_release( const arma::vec2& pos, const QMouseEvent& event) override;
  bool mouse_move(const arma::vec2& delta, const arma::vec2& pos, const QMouseEvent& e) override;

protected:
  ObjectTransformation transformation() const override;
  void set_selected(bool selected) override;
  void clear() override;
  bool is_selected() const override;

private:
  SelectTool<ObjectPositions>& m_tool;
  Scene& m_scene;
  Object& m_object;
};

class PointSelectHandle : public AbstractSelectHandle
{
public:
  enum class Tangent { Left, Right };
  enum class TangentMode { Mirror, Individual };
  explicit PointSelectHandle(SelectTool<PointPositions>& tool, Path& path, Point& point);
  bool contains(const arma::vec2& point) const override;
  void draw(omm::AbstractRenderer& renderer) const override;
  bool mouse_press( const arma::vec2& pos, const QMouseEvent& event) override;
  bool mouse_move(const arma::vec2& delta, const arma::vec2& pos, const QMouseEvent& e) override;
  void mouse_release( const arma::vec2& pos, const QMouseEvent& event) override;

  template<Tangent tangent>
  void transform_tangent(const arma::vec2& delta);

protected:
  ObjectTransformation transformation() const override;
  void set_selected(bool selected) override;
  void clear() override;
  bool is_selected() const override;

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
