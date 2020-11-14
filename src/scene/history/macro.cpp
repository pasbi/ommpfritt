#include "scene/history/macro.h"
#include "scene/scene.h"
#include <QUndoStack>

namespace omm
{
Macro::Macro(const QString& text, QUndoStack& stack) : m_undo_stack(stack)
{
  m_undo_stack.beginMacro(text);
}

Macro::~Macro()
{
  m_undo_stack.endMacro();
}

RememberSelectionMacro::RememberSelectionMacro(Scene& scene, const QString& text, QUndoStack& stack)
    : Macro(text, stack), m_scene(scene), m_selection(scene.selection())
{
}

RememberSelectionMacro::~RememberSelectionMacro()
{
  m_scene.set_selection(m_selection);
}

}  // namespace omm
