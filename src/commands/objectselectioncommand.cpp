#include "commands/objectselectioncommand.h"
#include "scene/scene.h"

namespace omm
{
ObjectSelectionCommand ::ObjectSelectionCommand(Scene& scene,
                                                const std::set<Object*>& new_object_selection)
    : Command(QObject::tr("Set Object Selection")), m_scene(scene),
      m_new_object_selection(new_object_selection),
      m_old_object_selection(m_scene.item_selection<Object>())
{
}

void ObjectSelectionCommand::undo()
{
  m_scene.set_selection(down_cast(m_old_object_selection));
}

void ObjectSelectionCommand::redo()
{
  m_scene.set_selection(down_cast(m_new_object_selection));
}

}  // namespace omm
