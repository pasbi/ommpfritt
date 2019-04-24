#include "keybindings/defaultkeysequenceparser.h"

#include <QFile>
#include <QTextStream>
#include "common.h"
#include "logging.h"

namespace
{

auto parse_sequences(const std::string& filename, const std::string& group)
{
  std::map<std::string, QKeySequence> map;
  QFile file(QString::fromStdString(filename));
  if (!file.open(QIODevice::ReadOnly)) {
    LERROR << "Failed to open file '" << filename << "'.";
    return map;
  }

  bool group_is_active = false;
  QTextStream stream(&file);
  while (!stream.atEnd()) {
    QString line = stream.readLine();
    line = line.trimmed();
    if (line.startsWith("#") || line.isEmpty()) {
      continue;  // comment
    }

    if (line == QString("[%1]").arg(group.c_str())) {
      group_is_active = true;
    } else if (line.startsWith("[")) {
      group_is_active = false;
    } else if (group_is_active) {
      const auto tokens = line.split(":");
      if (tokens.size() != 2) {
        LWARNING << "ignoring line '" << line.toStdString()
                     << "'. Expected format: <name>: <key sequence>.";
        continue;
      }
      const auto name = tokens[0].trimmed().toStdString();
      const auto code = tokens[1].trimmed();
      const auto sequence = QKeySequence(code);
      if (sequence.isEmpty() != code.isEmpty()) {
        LWARNING << "Failed to parse key sequence for '" << name
                     << "': '" << code.toStdString() << "'.";
      } else if (map.count(name) > 0) {
        LWARNING << "Duplicate key sequence for '" << name << ". Drop '"
                     << sequence.toString().toStdString() << "', keep '"
                     << map[name].toString().toStdString() << "'.";
      } else {
        map[name] = sequence;
      }
    }

  }
  return map;
}

}  // namespace

namespace omm
{

DefaultKeySequenceParser::DefaultKeySequenceParser(const std::string &filename, const std::string &group)
  : m_sequences(parse_sequences(filename, group))
{
}

QKeySequence DefaultKeySequenceParser::get_key_sequence(const std::string &name) const
{
  try {
    return m_sequences.at(name);
  } catch (const std::out_of_range&) {
    return QKeySequence();
  }
}

}  // namespace
