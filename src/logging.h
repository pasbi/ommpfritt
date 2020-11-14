#pragma once

#include "common.h"
#include <QDebug>
#include <QString>
#include <sstream>

#if defined(LDEBUG) || defined(LINFO) || defined(LERROR) || defined(LWARNING)
#  error Failed to define logging-macros due to name collision.
#endif

#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)

#define LDEBUG qDebug().nospace().noquote()
#define LINFO qInfo().nospace().noquote()
#define LWARNING qCritical().nospace().noquote()
#define LERROR qCritical().nospace().noquote()
#define LFATAL(...) qFatal(__VA_ARGS__)

class QFile;

namespace omm
{
namespace LogLevel
{
static constexpr auto DEBUG = "debug";
static constexpr auto INFO = "info";
static constexpr auto WARNING = "warning";
static constexpr auto CRITICAL = "critical";
static constexpr auto FATAL = "fatal";

const std::map<QString, int> loglevels{{omm::LogLevel::DEBUG, 0},
                                       {omm::LogLevel::INFO, 1},
                                       {omm::LogLevel::WARNING, 2},
                                       {omm::LogLevel::CRITICAL, 3},
                                       {omm::LogLevel::FATAL, 4}};

const std::map<QtMsgType, QString> printlevels{{QtDebugMsg, omm::LogLevel::DEBUG},
                                               {QtInfoMsg, omm::LogLevel::INFO},
                                               {QtWarningMsg, omm::LogLevel::WARNING},
                                               {QtCriticalMsg, omm::LogLevel::CRITICAL},
                                               {QtFatalMsg, omm::LogLevel::FATAL}};

}  // namespace LogLevel

void handle_log(QFile& logfile,
                const QString& level,
                bool print_long_message,
                QtMsgType type,
                const QMessageLogContext& ctx,
                const QString& msg);
void setup_logfile(QFile& logfile);

}  // namespace omm

QDebug operator<<(QDebug d, const std::string& string);

// this enables streaming to qDebug and friends for
// each type which can be streamed into std::ostream.
// disable this function for enums. It will create ambiguities.
// Qt-Enums are handled by the default qDebug very well. Other enums will fallback to int.
template<typename T>
std::enable_if_t<!std::is_enum_v<T>, QDebug> operator<<(QDebug ostream, const T& t)
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
