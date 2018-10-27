#include "commands/addobjectcommand.h"
#include "objects/object.h"
#include "scene/scene.h"

namespace {
std::string get_command_label(const omm::Object& object)
{
  std::ostringstream ostr;
  ostr << "Add " << object.class_name();
  return ostr.str();
}
}  // namespace

omm::AddObjectCommand::AddObjectCommand(omm::Scene& scene, std::unique_ptr<omm::Object> object)
  : SceneCommand(get_command_label(*object), scene)
  , m_owned_object(std::move(object))
  , m_object_reference(*m_owned_object)
{
}

void omm::AddObjectCommand::redo()
{
  if (!m_owned_object) {
    LOG(FATAL) << "Command cannot give away non-owned object.";
  } else {
    scene().root().adopt(std::move(m_owned_object));
  }
}

void omm::AddObjectCommand::undo()
{
  if (m_owned_object) {
    LOG(FATAL) << "Command already owns object. Obtaining ownership again is absurd.";
  } else {
    m_owned_object = scene().root().repudiate(m_object_reference);
  }
}

