#pragma once

#include <QDebug>
#include <QString>
#include <sstream>
#include "common.h"

#if defined(LDEBUG) || defined(LINFO) || defined(LERROR) || defined(LWARNING)
#error Failed to define logging-macros due to name collision.
#endif

namespace omm
{

struct LogPrefix
{
  explicit LogPrefix(const QString& level, const char* file, int line)
    : level(level), file(file), line(line) {}
  const QString level;
  const char* file;
  const int line;
};

QDebug operator<<(QDebug d, const LogPrefix& prefix);

}  // namespace omm

#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)

#define LDEBUG qDebug().nospace().noquote() << omm::LogPrefix("Debug", __FILE__, __LINE__)
#define LINFO qInfo().nospace().noquote() << omm::LogPrefix("Info", __FILE__, __LINE__)
#define LWARNING qCritical().nospace().noquote() << omm::LogPrefix("Warning", __FILE__, __LINE__)
#define LERROR qCritical().nospace().noquote() << omm::LogPrefix("Error", __FILE__, __LINE__)
#define LFATAL(...) qFatal(__VA_ARGS__)

QDebug operator<< (QDebug d, const std::string& string);

// this enables streaming to qDebug and friends for
// each type which can be streamed into std::ostream.
// disable this function for enums. It will create ambiguities.
// Qt-Enums are handled by the default qDebug very well. Other enums will fallback to int.
template<typename T> std::enable_if_t<!std::is_enum_v<T>, QDebug>
operator<<(QDebug ostream, const T& t)
{
  std::ostringstream oss;
  oss << t;
  ostream << QString::fromStdString(oss.str());
  return ostream;
}

template<template<typename> typename ContainerT, typename T>
void stream_container(QDebug& ostream, const ContainerT<T>& vs, const std::string container_name)
{
  ostream << container_name << "(" << vs.size() << ")[";
  for (auto it = vs.cbegin(); it != vs.cend(); ++it) {
    if (it != vs.cbegin()) {
      ostream << ", ";
    }
    ostream << *it;
  }
  ostream << "]";
}

template<typename T> QDebug operator<<(QDebug ostream, const std::vector<T>& vs)
{
  stream_container<std::vector, T>(ostream, vs, "vector");
  return ostream;
}

template<typename T> QDebug operator<<(QDebug ostream, const std::set<T>& vs)
{
  stream_container<std::set, T>(ostream, vs, "set");
  return ostream;
}

template<typename... T> QDebug operator<<(QDebug ostream, const std::unique_ptr<T...>& uptr)
{
  ostream << *uptr;
  return ostream;
}
