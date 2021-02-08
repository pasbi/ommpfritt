#include "stringinterpolation.h"
#include "logging.h"
#include <QRegExp>
#include <QStringList>

namespace
{
std::pair<QString, QString> split_key(const QString& key)
{
  const int colon_pos = key.indexOf(omm::StringInterpolation::SEPARATOR);
  if (colon_pos < 0) {
    return {key, ""};
  } else {
    return {key.mid(0, colon_pos), key.mid(colon_pos + 1)};
  }
}

/**
 * @brief format formats an integer
 * @param value the value to format.
 * @param format the format. The first character represents the fill character,
 *  the remaining characters are interpreted as the length of the result.
 *  E.g., format(3, "x4") yields "xxx3".
 * @return the formatted value or an empty string if the format is invalid.
 */
QString format(int value, const QString& format)
{
  if (format.isEmpty()) {
    return QString("%1").arg(value);
  } else {
    auto fill = format.front();
    bool ok = false;
    auto size = format.midRef(1).toInt(&ok);
    if (ok) {
      static constexpr auto base = 10;
      return QString("%1").arg(value, size, base, fill);
    } else {
      throw omm::StringInterpolation::InvalidFormatException{"Invalid format '" + format + "'."};
    }
  }
}

QString format(const QString& value, [[maybe_unused]] const QString& format)
{
  return value;
}

QString format(const omm::StringInterpolation::value_variant& value, const QString& format)
{
  return std::visit([format](auto&& v) { return ::format(v, format); }, value);
}

}  // namespace

namespace omm
{
StringInterpolation::StringInterpolation(const QString& pattern, const map_type& values)
    : m_string(pattern)
{
  for (auto&& [key, value] : values) {
    replace_all(compile_regexp(key), value);
  }
  if (m_string.contains(START) || m_string.contains(END)) {
    LWARNING << "Bad pattern: " << m_string;
    throw InvalidFormatException{"Unevaluated braces in pattern."};
  }
}

StringInterpolation::operator QString() const
{
  return m_string;
}

QRegExp StringInterpolation::compile_regexp(const QString& key)
{
  static const auto start = QRegExp::escape(QString(omm::StringInterpolation::START));
  static const auto end = QRegExp::escape(QString(omm::StringInterpolation::END));
  const auto pattern = start + QRegExp::escape(key) + ".*" + end;
  QRegExp regexp(pattern);
  regexp.setMinimal(true);
  return regexp;
}

void StringInterpolation::replace_all(const QRegExp& regexp, const value_variant& value)
{
  int offset = 0;
  while (offset >= 0) {
    offset = replace_first(regexp, value, offset);
  }
}

int StringInterpolation::replace_first(const QRegExp& regexp,
                                       const value_variant& value,
                                       int offset)
{
  offset = regexp.indexIn(m_string, offset);
  if (offset >= 0) {
    const auto captures = regexp.capturedTexts();
    assert(captures.size() == 1);
    const auto& capture = captures.front();
    const auto&& [rawkey, format] = split_key(capture.mid(1, capture.size() - 2));
    const auto replacement = ::format(value, format);
    m_string = m_string.replace(offset, regexp.matchedLength(), replacement);
    offset += replacement.size();
  }
  return offset;
}

}  // namespace omm
