#include "command.h"

Command::Command(const std::string& label)
  : m_label(label)
{
}

Command::~Command()
{

}

std::unique_ptr<Command> Command::merge(std::unique_ptr<Command> other)
{
  // do not merge.
  return other;
}
