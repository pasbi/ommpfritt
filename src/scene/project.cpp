#include "project.h"
#include <fstream>
#include "external/json.hpp"

omm::Project::Project()
{

}

omm::Project::~Project()
{

}

bool omm::Project::save_as(const std::string &filename)
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
    set_has_no_pending_changes();
    m_filename = filename;
    return true;
  } else {
    LOG(ERROR) << "Failed to open ofstream at '" << filename << "'";
    return false;
  }
}

bool omm::Project::load_from(const std::string &filename)
{
  std::ifstream ifstream(filename);
  if (ifstream) {
    nlohmann::json json(ifstream);
    if (m_scene.load(json)) {
      set_has_no_pending_changes();
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

void omm::Project::reset()
{
  m_scene.reset();
}

void omm::Project::submit(std::unique_ptr<omm::Command> command)
{
  CommandStack::submit(std::move(command));
}

std::string omm::Project::filename() const
{
  return m_filename;
}

omm::Scene& omm::Project::scene()
{
  return m_scene;
}