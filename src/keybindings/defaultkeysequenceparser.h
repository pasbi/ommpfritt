#pragma once

#include <map>
#include <QKeySequence>
#include <string>

namespace omm
{

class DefaultKeySequenceParser
{
public:
  DefaultKeySequenceParser(const std::string& filename, const std::string& group);
  QKeySequence get_key_sequence(const std::string& name) const;

private:
  const std::map<std::string, QKeySequence> m_sequences;
};

}  // namespace omm
