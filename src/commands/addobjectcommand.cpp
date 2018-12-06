#include "commands/addobjectcommand.h"
#include "objects/object.h"
#include "scene/scene.h"
#include "properties/stringproperty.h"

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
  : Command(get_command_label(*object))
  , m_owned(std::move(object))
  , m_reference(*m_owned)
  , m_scene(scene)
{
  static int i = 0;
  const auto name = m_reference.type() + " " + std::to_string(i++);
  m_reference.property<StringProperty>(AbstractPropertyOwner::NAME_PROPERTY_KEY).set_value(name);
}

void AddObjectCommand::redo()
{
  if (!m_owned) {
    LOG(FATAL) << "Command cannot give away non-owned object.";
  } else {
    m_scene.insert(std::move(m_owned), m_scene.root());
    m_scene.selection_changed();
  }
}

void AddObjectCommand::undo()
{
  if (m_owned) {
    LOG(FATAL) << "Command already owns object. Obtaining ownership again is absurd.";
  } else {
    m_owned = m_scene.root().repudiate(m_reference);

    // important. else, handle or property manager might point to dangling objects
    m_scene.selection_changed();
  }
}

}  // namespace omm

