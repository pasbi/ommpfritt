#include "scene/history/macro.h"
#include "scene/scene.h"
#include <QUndoStack>

namespace omm
{

Macro::Macro(const QString &text, QUndoStack &stack)
  : m_undo_stack(stack)
{
  m_undo_stack.beginMacro(text);
}

Macro::~Macro()
{
  m_undo_stack.endMacro();
  QUndoCommand* last_command = const_cast<QUndoCommand*>(m_undo_stack.command(m_undo_stack.count()-1));
  if (const int n = last_command->childCount(); n <= 2) {
    LWARNING << "detected macro with " << n << " commands.";
  }
}

RememberSelectionMacro::RememberSelectionMacro(Scene &scene, const QString &text, QUndoStack &stack)
  : Macro(text, stack)
  , m_scene(scene)
  , m_selection(scene.selection())
{
}

RememberSelectionMacro::~RememberSelectionMacro()
{
  m_scene.set_selection(m_selection);
}

}  // namespace omm
