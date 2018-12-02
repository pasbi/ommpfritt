#include "renderers/abstractrenderer.h"
#include "scene/scene.h"

namespace omm
{

AbstractRenderer::AbstractRenderer(const BoundingBox& bounding_box)
  : m_bounding_box(bounding_box)
{
}

void AbstractRenderer::render(const Scene& scene)
{
  scene.root().render_recursive(*this, scene.default_style());
}

const BoundingBox& AbstractRenderer::bounding_box() const
{
  return m_bounding_box;
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
    return m_base_transformation;
  } else {
    return m_transformation_stack.top();
  }
}

void AbstractRenderer::set_base_transformation(const ObjectTransformation& base_transformation)
{
  m_base_transformation = base_transformation;
}

}  // namespace omm
