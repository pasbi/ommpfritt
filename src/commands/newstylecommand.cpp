#include "commands/newstylecommand.h"
#include "scene/stylepool.h"

namespace omm
{

NewStyleCommand::NewStyleCommand(StylePool& style_pool, std::unique_ptr<Style> style)
  : Command(QObject::tr("New Style").toStdString())
  , m_style_pool(style_pool)
  , m_owned(std::move(style))
  , m_reference(*m_owned)
{
}

void NewStyleCommand::undo()
{
  assert(!m_owned);
  m_owned = m_style_pool.remove_style(m_reference);
}

void NewStyleCommand::redo()
{
  assert(m_owned);
  m_style_pool.insert_style(std::move(m_owned));
}


}  // namespace omm
