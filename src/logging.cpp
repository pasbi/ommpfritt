#include "logging.h"
#include <iomanip>
#include <ctime>

namespace omm
{

std::string current_time()
{
  std::ostringstream oss;
  const auto time = std::time(nullptr);
  const auto time_m = *std::localtime(&time);
  oss << std::put_time(&time_m, "%Y-%m-%d %H:%M:%S");
  return oss.str();
}

}  // namespace omm

QDebug operator<< (QDebug d, const std::string& string)
{
  d << QString::fromStdString(string);
  return d;
}
