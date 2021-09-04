#pragma once

#include "common.h"
#include <QDebug>
#include <QString>
#include <sstream>

#if defined(LDEBUG) || defined(LINFO) || defined(LERROR) || defined(LWARNING)
#  error Failed to define logging-macros due to name collision.
#endif

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define MAKE_Q_MESSAGE_LOGGER(level) \
  QMessageLogger(static_cast<const char*>(__FILE__), \
                 __LINE__, \
                 static_cast<const char*>(Q_FUNC_INFO)) \
      .level

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define MAKE_DECORATED_Q_MESSAGE_LOGGER(level) MAKE_Q_MESSAGE_LOGGER(level()).nospace().noquote()

#define LDEBUG MAKE_DECORATED_Q_MESSAGE_LOGGER(debug)  // NOLINT(cppcoreguidelines-macro-usage)
#define LINFO MAKE_DECORATED_Q_MESSAGE_LOGGER(info)  // NOLINT(cppcoreguidelines-macro-usage)
#define LWARNING MAKE_DECORATED_Q_MESSAGE_LOGGER(warning)  // NOLINT(cppcoreguidelines-macro-usage)
#define LERROR MAKE_DECORATED_Q_MESSAGE_LOGGER(critical)  // NOLINT(cppcoreguidelines-macro-usage)
#define LFATAL MAKE_Q_MESSAGE_LOGGER(fatal)  // NOLINT(cppcoreguidelines-macro-usage)

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
