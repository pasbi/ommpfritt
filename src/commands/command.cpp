#include "command.h"

omm::Command::Command(const std::string& label)
  : m_label(label)
{
}

omm::Command::~Command()
{

}

std::unique_ptr<omm::Command> omm::Command::merge(std::unique_ptr<omm::Command> other)
{
  // do not merge.
  return other;
}

std::string omm::Command::label() const
{
  return m_label;
}