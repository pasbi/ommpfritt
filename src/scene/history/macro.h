#pragma once

#include <set>

class QUndoStack;
class QString;

namespace omm
{

class AbstractPropertyOwner;
class Scene;
class Macro
{
public:
  explicit Macro(const QString& text, QUndoStack& stack);
  virtual ~Macro();
private:
  QUndoStack& m_undo_stack;
};

class RememberSelectionMacro : public Macro
{
public:
  explicit RememberSelectionMacro(Scene& scene, const QString& text, QUndoStack& stack);
  ~RememberSelectionMacro();

private:
  Scene& m_scene;
  std::set<AbstractPropertyOwner*> m_selection;
};

}  // namespace omm
