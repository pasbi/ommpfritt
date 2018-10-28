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

namespace omm
{

AddObjectCommand::AddObjectCommand(Project& project, std::unique_ptr<omm::Object> object)
  : Command(project, QString::fromStdString(get_command_label(*object)))
  , m_owned_object(std::move(object))
  , m_object_reference(*m_owned_object)
{
}

void AddObjectCommand::redo()
{
  if (!m_owned_object) {
    LOG(FATAL) << "Command cannot give away non-owned object.";
  } else {
    scene().insert_object(std::move(m_owned_object), scene().root());
  }
}

void AddObjectCommand::undo()
{
  if (m_owned_object) {
    LOG(FATAL) << "Command already owns object. Obtaining ownership again is absurd.";
  } else {
    m_owned_object = scene().root().repudiate(m_object_reference);
  }
}

}  // namespace omm

