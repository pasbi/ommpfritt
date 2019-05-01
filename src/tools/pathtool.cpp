#include "tools/pathtool.h"
#include "scene/scene.h"
#include <QMouseEvent>
#include "commands/addcommand.h"

namespace omm
{

PathTool::PathTool(Scene &scene) : Tool(scene)
{

}

bool PathTool::mouse_move(const Vec2f &delta, const Vec2f &pos, const QMouseEvent &event)
{
  Q_UNUSED(pos)
  Q_UNUSED(event)
  if (m_current_point != nullptr) {
    const auto lt = PolarCoordinates(m_current_point->left_tangent.to_cartesian() + delta);
    m_current_point->left_tangent = lt;
    m_current_point->right_tangent = -lt;
    return true;
  } else {
    return false;
  }
}

bool PathTool::mouse_press(const Vec2f &pos, const QMouseEvent &event, bool force)
{
  Q_UNUSED(force)
  switch (event.button()) {
  case Qt::LeftButton:
    add_point(pos);
    return true;
  case Qt::RightButton:
    end();
    return true;
  default:
    return false;
  }
}

void PathTool::mouse_release(const Vec2f &pos, const QMouseEvent &event)
{
  Q_UNUSED(pos)
  Q_UNUSED(event)
  m_current_point = nullptr;
}

std::string PathTool::type() const { return TYPE; }
void PathTool::cancel() { m_path.reset(); }

void PathTool::draw(AbstractRenderer &renderer) const
{
  Tool::draw(renderer);
  if (m_path) {
    m_path->set_transformation(viewport_transformation);
    m_path->draw_recursive(renderer, scene.default_style());
  }
}

void PathTool::add_point(const Vec2f &pos)
{
  if (!m_path) {
    m_path = std::make_unique<Path>(&scene);
  }

  const auto gpos = viewport_transformation.inverted().apply_to_position(pos);

  Path::PointSequence point_sequence;
  point_sequence.position = m_path->points().size();
  point_sequence.sequence = { Point(gpos) };
  m_path->add_points(std::vector { point_sequence });
  m_current_point = m_path->points_ref().back();
}

void PathTool::end()
{
  if (m_path) {
    auto t = ObjectTransformation();
    using add_command_type = AddCommand<Tree<Object>>;
    for (Point* point : m_path->points_ref()) {
      *point = t.apply(*point);
    }
    m_path->set_global_transformation(t.inverted(), false);
    m_path->property(Path::INTERPOLATION_PROPERTY_KEY)->set(Path::InterpolationMode::Bezier);
    scene.submit<add_command_type>(scene.object_tree, std::move(m_path));
  }
}

}  // namespace
