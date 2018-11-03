#include "project.h"
#include <fstream>
#include "external/json.hpp"
#include "commands/command.h"

namespace omm
{

Project::Project()
  : m_scene(*this)
{
}

Project::~Project()
{
}

bool Project::save_as(const std::string &filename)
{
  const auto json = m_scene.save();
  std::ofstream ofstream(filename);
  if (ofstream) {
#ifdef NDEBUG
    ofstream << json;
#else
    ofstream << json.dump(2);
#endif
    LOG(INFO) << "Saved current scene to '" << filename << "'";
    set_has_pending_changes(false);
    m_filename = filename;
    return true;
  } else {
    LOG(ERROR) << "Failed to open ofstream at '" << filename << "'";
    return false;
  }
}

bool Project::load_from(const std::string &filename)
{
  std::ifstream ifstream(filename);
  if (ifstream) {
    nlohmann::json json(ifstream);
    if (m_scene.load(json)) {
      set_has_pending_changes(false);
      m_filename = filename;
      return true;
    } else {
      LOG(ERROR) << "Bad format: '" << filename << "'.";
      return false;
    }
  } else {
    LOG(ERROR) << "Failed to open '" << filename << "'.";
    return false;
  }
}

void Project::reset()
{
  set_has_pending_changes(false);
  m_scene.reset();
}

std::string Project::filename() const
{
  return m_filename;
}

Scene& Project::scene()
{
  return m_scene;
}

void Project::set_has_pending_changes(bool v)
{
  m_has_pending_changes = v;
}

bool Project::has_pending_changes() const
{
  return m_has_pending_changes;
}

void Project::submit(std::unique_ptr<Command> command)
{
  m_undo_stack.push(command.release());
}

QUndoStack& Project::undo_stack()
{
  return m_undo_stack;
}

}  // namespace omm
