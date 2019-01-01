#include "tools/tool.h"

#include <memory>
#include "renderers/abstractrenderer.h"
#include "objects/object.h"
#include "properties/optionsproperty.h"

namespace
{

arma::vec2 get_global_position_mean(const std::set<omm::Object*>& objects)
{
  assert(objects.size() > 0);
  const auto add = [](const arma::vec2& accu, const omm::Object* object) -> arma::vec2 {
    return accu + object->global_transformation().translation();
  };
  const auto null = arma::vec2 {0.0, 0.0};
  return std::accumulate(objects.begin(), objects.end(), null, add) / objects.size();
}

}  // namespace

namespace omm
{

Tool::Tool(Scene& scene, std::vector<std::unique_ptr<Handle>> handles)
  : scene(scene)
  , m_handles(std::move(handles))
{
  add_property<OptionsProperty>(ALIGNMENT_PROPERTY_KEY)
    .set_options({ "global", "local" })
    .set_label(QObject::tr("Alignment").toStdString())
    .set_category(QObject::tr("Tool").toStdString());
}

bool Tool::mouse_move(const arma::vec2& delta, const arma::vec2& pos) { return false; }
bool Tool::mouse_press(const arma::vec2& pos) { return false; }
void Tool::mouse_release() { }

ObjectTransformation Tool::transformation() const
{
  ObjectTransformation transformation;
  transformation.translate(get_global_position_mean(selection()));
  if (property(ALIGNMENT_PROPERTY_KEY).value<size_t>() == 1 && selection().size() == 1) {
      transformation.rotate((*selection().begin())->global_transformation().rotation());
  }
  return transformation;
}

void Tool::draw(AbstractRenderer& renderer) const
{
  if (selection().size() == 0) {
    return;
  }

  renderer.push_transformation(this->transformation());

  for (const auto* handle : handles()) {
    handle->draw(renderer);
  }
  renderer.pop_transformation();
}

void Tool::set_selection(const std::set<Object*>& objects)
{
  m_selection = objects;
}

std::vector<Handle*> Tool::handles() const
{
  return ::transform<Handle*>(m_handles, std::mem_fn(&std::unique_ptr<Handle>::get));
}

const std::set<Object*> Tool::selection() const
{
  return m_selection;
}

arma::vec2 Tool::map_to_tool_local(const arma::vec2& pos) const
{
  return transformation().inverted().apply_to_position(pos);
}


}  // namespace omm
