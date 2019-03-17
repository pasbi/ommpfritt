#include "renderers/abstractrenderer.h"
#include "scene/scene.h"

namespace omm
{

AbstractRenderer::AbstractRenderer(Scene& scene, Category filter)
  : scene(scene), m_enabled_categories(filter)
{
}

void AbstractRenderer::render()
{
  scene.object_tree.root().render_recursive(*this, scene.default_style());
  assert(m_transformation_stack.empty());
}

void AbstractRenderer::push_transformation(const ObjectTransformation& transformation)
{
  m_transformation_stack.push(current_transformation().apply(transformation));
}

void AbstractRenderer::pop_transformation()
{
  m_transformation_stack.pop();
}

ObjectTransformation AbstractRenderer::current_transformation() const
{
  if (m_transformation_stack.size() == 0) {
    return ObjectTransformation();
  } else {
    return m_transformation_stack.top();
  }
}

void AbstractRenderer::set_category(Category category) { m_current_category = category; }
bool AbstractRenderer::is_active() const { return !!(m_current_category & m_enabled_categories); }

}  // namespace omm
