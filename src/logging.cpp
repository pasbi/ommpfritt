#include "logging.h"
#include <iomanip>
#include <ctime>

namespace
{

#ifdef LOG_TIMESTAMP
std::string current_time()
{
  std::ostringstream oss;
  const auto time = std::time(nullptr);
  const auto time_m = *std::localtime(&time);
  oss << std::put_time(&time_m, "%Y-%m-%d %H:%M:%S");
  return oss.str();
}
#endif

}  // namespace

namespace omm
{

QDebug operator<<(QDebug d, const LogPrefix& prefix)
{
  Q_UNUSED(prefix)

#ifdef LOG_LEVEL
  d << prefix.level;
#endif
#ifdef LOG_LOCATION
  d << " @" << prefix.file << ":" << std::to_string(prefix.line);
#endif
#ifdef LOG_TIMESTAMP
  d << " [" << QString::fromStdString(current_time()) << "]";
#endif
#if defined(LOG_LEVEL) || defined(LOG_LOCATION) || defined(LOG_TIMESTAMP)
  d << ": ";
#endif
  return d;
}

}  // namespace omm

QDebug operator<< (QDebug d, const std::string& string)
{
  d << QString::fromStdString(string);
  return d;
}
