#include "commands/newstylecommand.h"
#include "scene/scene.h"

namespace omm
{

NewStyleCommand::NewStyleCommand(Scene& scene, std::unique_ptr<Style> style)
  : Command(QObject::tr("New Style").toStdString())
  , m_scene(scene)
  , m_owned(std::move(style))
  , m_reference(*m_owned)
{
  static int i = 0;
  const auto name = "Style " + std::to_string(i++);
  m_reference.property<StringProperty>(AbstractPropertyOwner::NAME_PROPERTY_KEY).set_value(name);
}

void NewStyleCommand::undo()
{
  assert(!m_owned);
  m_owned = m_scene.remove(m_reference);
}

void NewStyleCommand::redo()
{
  assert(m_owned);
  m_scene.insert(std::move(m_owned));
}


}  // namespace omm
