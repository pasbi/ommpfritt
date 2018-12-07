#include "commands/newstylecommand.h"
#include "scene/list.h"

namespace omm
{

NewStyleCommand::NewStyleCommand(List<Style>& structure, std::unique_ptr<Style> style)
  : Command(QObject::tr("New Style").toStdString())
  , m_structure(structure)
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
  m_owned = m_structure.remove(m_reference);
}

void NewStyleCommand::redo()
{
  assert(m_owned);
  m_structure.insert(std::move(m_owned));
}


}  // namespace omm
