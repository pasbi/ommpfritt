#include "commands/addobjectcommand.h"
#include "objects/object.h"
#include "scene/scene.h"
#include "properties/typedproperty.h"

namespace
{

std::string get_command_label(const omm::Object& object)
{
  return QObject::tr("Add %1").arg(object.type().c_str()).toStdString();
}

}  // namespace

namespace omm
{

AddObjectCommand::AddObjectCommand(Scene& scene, std::unique_ptr<omm::Object> object)
  : Command(scene, get_command_label(*object))
  , m_owned_object(std::move(object))
  , m_object_reference(*m_owned_object)
{
  static int i = 0;
  const auto name = std::string(1, char('a' + (i++)));
  m_owned_object->property(Object::NAME_PROPERTY_KEY).cast<std::string>().set_value(name);
}

void AddObjectCommand::redo()
{
  if (!m_owned_object) {
    LOG(FATAL) << "Command cannot give away non-owned object.";
  } else {
    scene.insert_object(std::move(m_owned_object), scene.root());
  }
}

void AddObjectCommand::undo()
{
  if (m_owned_object) {
    LOG(FATAL) << "Command already owns object. Obtaining ownership again is absurd.";
  } else {
    m_owned_object = scene.root().repudiate(m_object_reference);
  }
}

}  // namespace omm

