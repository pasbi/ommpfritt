#pragma once

#include "geometry/vec2.h"
#include "tools/handles/handle.h"
#include "tools/tool.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPen>

namespace omm
{
template<typename ToolT, AxisHandleDirection direction> class ScaleAxisHandle : public Handle
{
public:
  ScaleAxisHandle(ToolT& tool) : Handle(tool), m_direction(-Handle::axis_directions.at(direction))
  {
  }

  [[nodiscard]] bool contains_global(const Vec2f& point) const override
  {
    const Vec2f global_point = this->tool.transformation().inverted().apply_to_position(point);
    Vec2f v = project_onto_axis(global_point);

    // clamp v between o and m_direction
    v = v.clamped(Vec2f::min(Vec2f::o(), m_direction), Vec2f::max(Vec2f::o(), m_direction));

    return (global_point - v).euclidean_norm() < this->interact_epsilon();
  }

  bool mouse_press(const Vec2f& pos, const QMouseEvent& e) override
  {
    m_transformation = tool.transformation();
    return Handle::mouse_press(pos, e);
  }

  bool mouse_move(const Vec2f& delta, const Vec2f& pos, const QMouseEvent& e) override
  {
    Handle::mouse_move(delta, pos, e);
    if (this->status() == HandleStatus::Active) {
      const auto inv_tool_transformation = m_transformation.inverted();
      const Vec2f s = [this, inv_tool_transformation, pos, &e] {
        if (!!(e.modifiers() & Qt::ControlModifier)) {
          auto total_delta = inv_tool_transformation.apply_to_direction(pos - this->press_pos());
          static constexpr double STEP = 10.0;
          static const auto s = [](double t) {
            static constexpr double TOTAL_WHEEL_STEPS = 120.0;
            return std::pow(2.0, t / TOTAL_WHEEL_STEPS);
          };

          total_delta = -this->discretize(this->project_onto_axis(total_delta), true, STEP);
          return Vec2f{s(total_delta.x), s(total_delta.y)};
        } else {
          const auto global_pos = inv_tool_transformation.apply_to_position(pos);
          const auto origin = inv_tool_transformation.apply_to_position(this->press_pos());
          auto s = project_onto_axis(global_pos - origin) / origin;
          static constexpr double STEP = 0.2;
          s = this->discretize(s, true, STEP) + Vec2f{1, 1};
          for (auto i : {0u, 1u}) {
            if (constexpr auto eps = 10e-10; std::abs(s[i]) < eps) {
              s[i] = std::copysign(eps, s[i]);
            }
          }
          return s;
        }
      }();
      auto transformation = omm::ObjectTransformation().scaled(s);
      transformation = transformation.transformed(inv_tool_transformation);
      static_cast<ToolT&>(this->tool).transform_objects(transformation);
      const auto tool_info = QString("%1, %2").arg(s.x).arg(s.y);
      static_cast<ToolT&>(this->tool).tool_info = tool_info;
      return true;
    } else {
      return false;
    }
  }

  void draw(QPainter& painter) const override
  {
    painter.setTransform(this->tool.transformation().to_qtransform(), true);

    painter.save();
    const QString name = Handle::axis_names.at(direction);
    QPen pen;
    pen.setColor(this->ui_color(name + "-outline"));
    static constexpr double PEN_WIDTH = 2.0;
    pen.setWidthF(PEN_WIDTH);
    painter.setPen(pen);
    painter.drawLine(QPointF{0, 0}, m_direction.to_pointf());
    const auto size = Vec2f{1.0, 1.0} * 0.1 * m_direction.euclidean_norm();
    const QRectF rect((m_direction - size / 2.0).to_pointf(), QSizeF(size.x, size.y));
    painter.fillRect(rect, this->ui_color(name + "-fill"));
    painter.drawRect(rect);
    painter.restore();
  }

private:
  const Vec2f m_direction;
  ObjectTransformation m_transformation;

  [[nodiscard]] Vec2f project_onto_axis(const Vec2f& vec) const
  {
    const Vec2f s = m_direction;

    // project v onto the line through o and s
    return Vec2f::dot(vec, s) / Vec2f::dot(s, s) * s;
  }
};

}  // namespace omm
