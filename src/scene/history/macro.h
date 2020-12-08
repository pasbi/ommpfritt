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
  Macro(Macro&&) = delete;
  Macro(const Macro&) = delete;
  Macro& operator=(const Macro&) = delete;
  Macro& operator=(Macro&&) = delete;

private:
  QUndoStack& m_undo_stack;
};

class RememberSelectionMacro : public Macro
{
public:
  explicit RememberSelectionMacro(Scene& scene, const QString& text, QUndoStack& stack);
  ~RememberSelectionMacro() override;
  RememberSelectionMacro(RememberSelectionMacro&&) = delete;
  RememberSelectionMacro(const RememberSelectionMacro&) = delete;
  RememberSelectionMacro& operator=(RememberSelectionMacro&&) = delete;
  RememberSelectionMacro& operator=(const RememberSelectionMacro&) = delete;

private:
  Scene& m_scene;
  std::set<AbstractPropertyOwner*> m_selection;
};

}  // namespace omm
