#pragma once

#include <string>
#include <memory>
#include <QUndoStack>

#include "scene/scene.h"

namespace omm
{

class Command;

class Project
{
public:
  Project();
  ~Project();

  bool save_as(const std::string& filename);
  bool load_from(const std::string& filename);
  void reset();

  std::string filename() const;
  Scene& scene();
  void submit(std::unique_ptr<Command> command);

  bool has_pending_changes() const;
  QUndoStack& undo_stack();

private:
  void set_has_pending_changes(bool v);
  Scene m_scene;

  /**
   * holds the last filename this project was associated to.
   * is set in `save_as` and `load_from`
   */
  std::string m_filename;
  bool m_has_pending_changes = false;
  QUndoStack m_undo_stack;
};

}  // namespace omm
