#include "tools/transformpointshelper.h"
#include "cache.h"
#include "cachedgetter.h"
#include "geometry/objecttransformation.h"
#include "objects/path.h"
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

std::unique_ptr<PointsTransformationCommand>
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

  PointsTransformationCommand::Map map;

  bool is_noop = true;
  for (auto&& [key, point] : m_initial_points) {
    const ObjectTransformation premul = cache.get(key.path);
    auto p = premul.apply(point);
    if (p.is_selected != point.is_selected) {
      p.is_selected = point.is_selected;
      is_noop = false;
    }
    map.insert(std::pair(key, p));
  }

  if (!is_noop) {
    return std::make_unique<PointsTransformationCommand>(map);
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
  for (Path* path : m_paths) {
    for (PathIterator it = path->begin(); it != path->end(); ++it) {
      if (it->is_selected) {
        m_paths.insert(path);
        m_initial_points.insert({it, *it});
      }
    }
  }
  Q_EMIT initial_transformations_changed();
}
}   // namespace omm
