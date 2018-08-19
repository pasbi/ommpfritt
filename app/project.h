#pragma once

#include <string>
#include <memory>

#include "scene.h"
#include "commandstack.h"

class Project : public CommandStack
{
public:
  Project();
  ~Project();

  bool save_as(const std::string& filename);
  bool load_from(const std::string& filename);
  void reset();

  bool has_pending_changes() const;
  void submit(std::unique_ptr<Command> command) override;

  std::string filename() const;

private:
  Scene m_scene;

  /**
   * holds the last filename this project was associated to.
   * is set in `save_as` and `load_from`
   */
  std::string m_filename;
  bool m_has_pending_changes;


};