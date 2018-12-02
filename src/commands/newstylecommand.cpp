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
}

void NewStyleCommand::undo()
{
  assert(!m_owned);
  m_owned = m_scene.remove_style(m_reference);
}

void NewStyleCommand::redo()
{
  assert(m_owned);
  m_scene.insert_style(std::move(m_owned));
}


}  // namespace omm
