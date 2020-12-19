#include "logging.h"
#include "ommpfrittconfig.h"
#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <ctime>
#include <iomanip>
#include <iostream>

QDebug operator<<(QDebug d, const std::string& string)
{
  d << QString::fromStdString(string);
  return d;
}

namespace omm
{
void setup_logfile(QFile& logfile)
{
  const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  if (!QDir().mkpath(dir)) {
    std::cerr << "Failed to create directory " << dir.toStdString() << ".\n";
    std::cerr << std::flush;
    exit(2);
  }

  // Remove colon from timestamp, as colons are not supported in windows filenames.
  // Do the same on other platforms to ensure harmony.
  const QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate).replace(":", "");
  const QString filename = QString("%1/%2-log_%3.txt").arg(dir, qAppName(), timestamp);
  logfile.setFileName(filename);
  if (!logfile.open(QIODevice::WriteOnly)) {
    std::cerr << "Failed to open log file " << filename.toStdString() << " for writing.\n";
    std::cerr << std::flush;
    exit(2);
  } else {
    std::cout << "Using log file " << filename.toStdString() << "\n";
    std::cout << std::flush;
  }

  static const auto hex = [](const QByteArray& data) { return QString(data.toHex()); };

  logfile.write(QString("Log of the %1 application.\n").arg(qAppName()).toUtf8());
  logfile.write(QString("Application started at %1.\n").arg(timestamp).toUtf8());
  logfile.write(QString("boot unique id:   %1\n").arg(hex(QSysInfo::bootUniqueId())).toUtf8());
  logfile.write(QString("build ABI:        %1\n").arg(QSysInfo::buildAbi()).toUtf8());
  logfile.write(QString("build CPU arch:   %1\n").arg(QSysInfo::buildCpuArchitecture()).toUtf8());
  logfile.write(QString("current CPU arch: %1\n").arg(QSysInfo::currentCpuArchitecture()).toUtf8());
  logfile.write(QString("kernel type:      %1\n").arg(QSysInfo::kernelType()).toUtf8());
  logfile.write(QString("kernel version:   %1\n").arg(QSysInfo::kernelVersion()).toUtf8());
  logfile.write(QString("host name:        %1\n").arg(QSysInfo::machineHostName()).toUtf8());
  logfile.write(QString("machine id:       %1\n").arg(hex(QSysInfo::bootUniqueId())).toUtf8());
  logfile.write(QString("product name:     %1\n").arg(QSysInfo::prettyProductName()).toUtf8());
  logfile.write(QString("product type:     %1\n").arg(QSysInfo::productType()).toUtf8());
  logfile.write(QString("product version:  %1\n").arg(QSysInfo::productVersion()).toUtf8());
  logfile.write("\n");
  logfile.flush();
}

void handle_log(QFile& logfile,
                const QString& level,
                bool print_long_message,
                QtMsgType type,
                const QMessageLogContext& ctx,
                const QString& msg)
{
  using namespace LogLevel;
  const auto timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
  const QString rel_fn = QDir(source_directory).relativeFilePath(ctx.file);
  const auto long_message = QString("[%1] %2 %3:%4: %5\n")
                                .arg(printlevels.at(type), timestamp, rel_fn)
                                .arg(ctx.line)
                                .arg(msg);
  if (loglevels.at(printlevels.at(type)) >= loglevels.at(level)) {
    if (print_long_message) {
      fprintf(stderr, "%s", long_message.toUtf8().constData());
    } else {
      fprintf(stderr, "%s\n", msg.toUtf8().constData());
    }
  }

  logfile.write(long_message.toUtf8().constData());
  if (printlevels.at(type) >= loglevels.at(LogLevel::CRITICAL)) {
    logfile.flush();
  }
}

}  // namespace omm
