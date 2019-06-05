#pragma once

#include "geometry/vec2.h"
#include "tools/handles/handle.h"
#include "tools/handles/particlehandle.h"

namespace omm
{

class Scene;
class Path;

class AbstractSelectHandle : public Handle
{
public:
  explicit AbstractSelectHandle(Tool& tool);
  bool mouse_press(const Vec2f& pos, const QMouseEvent &event, bool force) override;
  bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e) override;

protected:
  virtual void set_selected(bool selected) = 0;
  virtual void clear() = 0;
  virtual bool is_selected() const = 0;

  static constexpr auto extend_selection_modifier = Qt::ShiftModifier;
};

class ObjectSelectHandle : public AbstractSelectHandle
{
public:
  explicit ObjectSelectHandle(Tool& tool, Scene& scene, Object& object);
  bool contains_global(const Vec2f& point) const override;
  void draw(omm::Painter& renderer) const override;

protected:
  ObjectTransformation transformation() const override;
  void set_selected(bool selected) override;
  void clear() override;
  bool is_selected() const override;

private:
  Scene& m_scene;
  Object& m_object;
};

class PointSelectHandle : public AbstractSelectHandle
{
public:
  enum class Tangent { Left, Right };
  enum class TangentMode { Mirror, Individual };
  explicit PointSelectHandle(Tool& tool, Path& path, Point& point);
  bool contains_global(const Vec2f& point) const override;
  void draw(omm::Painter& renderer) const override;
  bool mouse_press( const Vec2f& pos, const QMouseEvent& event, bool force) override;
  bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e) override;
  void mouse_release( const Vec2f& pos, const QMouseEvent& event) override;

  template<Tangent tangent>
  void transform_tangent(const Vec2f& delta);
  bool force_draw_subhandles = false;

protected:
  ObjectTransformation transformation() const override;
  void set_selected(bool selected) override;
  void clear() override;
  bool is_selected() const override;

private:
  Path& m_path;
  Point& m_point;
  const std::unique_ptr<Style> m_tangent_style;
  std::unique_ptr<ParticleHandle> m_left_tangent_handle;
  std::unique_ptr<ParticleHandle> m_right_tangent_handle;
  bool tangents_active() const;

  template<Tangent tangent>
  void transform_tangent(const Vec2f& delta, TangentMode mode);
};



}  // namespace omm
