#include "commands/removestylescommand.h"
#include "scene/scene.h"
#include "renderers/style.h"

namespace
{

// TODO same as RemoveObjectsCommand.cpp:make_contextes()
auto make_contextes(omm::Scene& scene, const std::set<omm::Style*>& selection)
{
  std::vector<omm::OwningListContext<omm::Style>> contextes;
  contextes.reserve(selection.size());
  for (auto style : selection) {
    size_t position = scene.position(*style);
    contextes.emplace_back(*style, position == 0 ? nullptr : &scene.style(position-1));
  }

  // assert that to-be-inserted objects' predecessor is already in the tree,
  // i.e., insert the predecessor first.
  std::sort(contextes.begin(), contextes.end(), [](const auto& lhs, const auto& rhs) {
    if (lhs.predecessor == &rhs.subject.reference()) {
      return true;
    } else {
      return false;
    }
  });
  return contextes;
}

}  // namespace

namespace omm
{

RemoveStylesCommand::RemoveStylesCommand(Scene& scene, const std::set<omm::Style*>& styles)
  : Command(QObject::tr("remove").toStdString())
  , m_scene(scene), m_contextes(make_contextes(scene, styles))
{
}

void RemoveStylesCommand::undo()
{
  for (auto&& it = m_contextes.rbegin(); it != m_contextes.rend(); ++it) {
    assert(it->subject.owns());
    auto& context = *it;

    // if predecessor is not null, it must had been inserted in the object tree.
    m_scene.insert_style(context);
  }
  m_scene.selection_changed();
}

void RemoveStylesCommand::redo()
{
  for (auto&& context : m_contextes) {
    assert(!context.subject.owns());
    assert(m_scene.find_reference_holders(context.subject).size() == 0);
    m_scene.remove_style(context);
  }
  // important. else, handle or property manager might point to dangling objects
  m_scene.selection_changed();
}

}  // namespace omm