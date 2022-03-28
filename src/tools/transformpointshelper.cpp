#include "tools/transformpointshelper.h"
#include "cache.h"
#include "cachedgetter.h"
#include "geometry/objecttransformation.h"
#include "objects/pathobject.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"
#include "scene/mailbox.h"
#include "scene/scene.h"

namespace omm
{

TransformPointsHelper::TransformPointsHelper(Scene& scene, Space space)
    : m_scene(scene), m_space(space)
{
  update();
  connect(&m_scene.mail_box(), &MailBox::point_selection_changed, this, qOverload<>(&TransformPointsHelper::update));
  connect(&m_scene.mail_box(), &MailBox::object_selection_changed, this, qOverload<>(&TransformPointsHelper::update));
  connect(&m_scene.mail_box(), &MailBox::object_removed, this, [this]() { update({}); });
}

std::unique_ptr<ModifyPointsCommand>
TransformPointsHelper::make_command(const ObjectTransformation& t) const
{
  class TransformationCache : public Cache<PathObject*, ObjectTransformation>
  {
  public:
    TransformationCache(const Matrix& mat, Space space) : m_mat(mat), m_space(space)
    {
    }

    ObjectTransformation retrieve(PathObject* const& path) const override
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
    const ObjectTransformation premul = cache.get(ptr->path_vector()->path_object());
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

void TransformPointsHelper::update(const std::set<PathObject*>& path_objects)
{
  m_path_objects = path_objects;
  update();
}

void TransformPointsHelper::update()
{
  m_initial_points.clear();
  for (const auto* path_object : m_path_objects) {
    for (PathPoint* point : path_object->geometry().selected_points()) {
      m_initial_points[point] = point->geometry();
      for (PathPoint* buddy : point->joined_points()) {
        m_initial_points[buddy] = point->compute_joined_point_geometry(*buddy);
      }
    }
  }
  Q_EMIT initial_transformations_changed();
}

}   // namespace omm
