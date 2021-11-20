#include "tools/transformpointshelper.h"
#include "cache.h"
#include "cachedgetter.h"
#include "geometry/objecttransformation.h"
#include "objects/path.h"
#include "objects/pathpoint.h"
#include "scene/mailbox.h"
#include "scene/scene.h"

namespace omm
{

TransformPointsHelper::TransformPointsHelper(Scene& scene, Space space)
    : m_scene(scene), m_space(space)
{
  update();
  connect(&m_scene.mail_box(),
          &MailBox::point_selection_changed,
          this,
          qOverload<>(&TransformPointsHelper::update));
}

std::unique_ptr<ModifyPointsCommand>
TransformPointsHelper::make_command(const ObjectTransformation& t) const
{
  class TransformationCache : public Cache<Path*, ObjectTransformation>
  {
  public:
    TransformationCache(const Matrix& mat, Space space) : m_mat(mat), m_space(space)
    {
    }

    ObjectTransformation retrieve(Path* const& path) const override
    {
      const Matrix gt = path->global_transformation(m_space).to_mat();
      return ObjectTransformation(gt.inverted() * m_mat * gt);
    }

  private:
    const Matrix m_mat;
    const Space m_space;
  };

  assert(!t.has_nan());
  assert(!t.to_mat().has_nan());
  TransformationCache cache(t.to_mat(), m_space);

  std::map<PathPoint*, Point> map;
  bool is_noop = true;
  for (auto&& [ptr, initial_value] : m_initial_points) {
    const ObjectTransformation premul = cache.get(ptr->path());
    auto p = premul.apply(initial_value);
    is_noop = false;
    map[ptr] = p;
  }

  if (!is_noop) {
    return std::make_unique<ModifyPointsCommand>(map);
  } else {
    return nullptr;
  }
}

void TransformPointsHelper::update(const std::set<Path*>& paths)
{
  m_paths = paths;
  update();
}

void TransformPointsHelper::update()
{
  m_initial_points.clear();
  for (auto* path : m_paths) {
    for (PathPoint* point : path->selected_points()) {
      m_initial_points[point] = point->geometry();
      for (PathPoint* buddy : point->joined_points()) {
        auto geometry = buddy->geometry();
        geometry.set_position(point->geometry().position());
        m_initial_points[buddy] = geometry;
      }
    }
  }
  Q_EMIT initial_transformations_changed();
}

}   // namespace omm
