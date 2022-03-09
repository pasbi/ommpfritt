#include "logging.h"
#include "config.h"
#include "main/application.h"
#include "main/commandlineparser.h"
#include "main/options.h"
#include "python/pythonengine.h"
#include <QApplication>
#include <QDirIterator>
#include <QSettings>
#include <QVariant>
#include <iostream>

const bool print_long_message = true;

// The logfile must be non-const and globally accessible, because the message handler lambda
// must not capture anything (see qInstallMessageHandler).
QFile logfile;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

// The level must be non-const and globally accessible, because the message handler lambda
// must not capture anything (see qInstallMessageHandler).
QString level = "";  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

namespace omm
{
int gui_main(const CommandLineParser& args, Application& app);
int render_main(const CommandLineParser& args, Application& app);
int tree_main(const CommandLineParser& args, Application& app);
}  // namespace omm

int main(int argc, char* argv[])
{
  using namespace omm;
  QApplication qt_app(argc, argv);

  QApplication::setOrganizationName("ommpfritt developers");
  QApplication::setApplicationName("ommpfritt");
  QApplication::setApplicationVersion(git_describe().data());
  QApplication::setApplicationDisplayName("ommpfritt");

  CommandLineParser args(QApplication::arguments());

  level = args.get<QString>(CommandLineParser::VERBOSITY_KEY);
  if (!omm::LogLevel::loglevels.contains(level)) {
    const auto levels = util::transform<QList>(::get_keys(omm::LogLevel::loglevels));
    std::cerr << "Unknown log level '" << level.toStdString()
              << "'. Use " << levels.join("|").toStdString() << ".\n"
              << std::flush;
    exit(EXIT_FAILURE);
  }

  omm::setup_logfile(logfile);
  qInstallMessageHandler([](QtMsgType type, const QMessageLogContext& ctx, const QString& msg) {
    omm::handle_log(logfile, level, print_long_message, type, ctx, msg);
  });

  const auto mode = args.get<QString>(CommandLineParser::MODE_KEY);
  auto options = std::make_unique<omm::Options>(
       mode != CommandLineParser::GUI_MODE_NAME,
       !args.is_set(CommandLineParser::NO_OPENGL_KEY)
  );

  PythonEngine::instance();
  omm::Application app(qt_app, std::move(options));

  if (mode == CommandLineParser::GUI_MODE_NAME) {
    return omm::gui_main(args, app);
  } else if (mode == CommandLineParser::TREE_MODE_NAME) {
    return omm::tree_main(args, app);
  } else if (mode == CommandLineParser::RENDER_MODE_NAME) {
    return omm::render_main(args, app);
  } else {
    std::cerr << "Unexpected mode: " << mode.toStdString() << "." << std::endl;
    return EXIT_FAILURE;
  }
}
