#include "project.h"
#include <fstream>
#include "external/json.hpp"

Project::Project()
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
    ofstream << json;
    LOG(INFO) << "Saved current scene to '" << filename << "'";
    m_has_pending_changes = false;
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
      m_has_pending_changes = false;
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
  m_scene.reset();
}

bool Project::has_pending_changes() const
{
  return m_has_pending_changes;
}

void Project::submit(std::unique_ptr<Command> command)
{
  m_has_pending_changes = true;
  CommandStack::submit(std::move(command));
}

std::string Project::filename() const
{
  return m_filename;
}