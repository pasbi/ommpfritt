#include "logging.h"
#include "mainwindow/application.h"
#include "mainwindow/mainwindow.h"
#include "mainwindow/iconprovider.h"
#include "qapplication.h"
#include "serializers/abstractserializer.h"
#include "tools/selectobjectstool.h"
#include "tools/toolbox.h"
#include <QApplication>
#include <QDirIterator>
#include <QSettings>
#include <QVariant>
#include <iostream>

const QString level = "debug";
const bool print_long_message = true;

// The logfile must be non-const and globally accessible, because the message handler lambda
// must not capture anything (see qInstallMessageHandler).
QFile logfile;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

int main(int argc, char* argv[])
{
  QApplication qt_app(argc, argv);

  omm::setup_logfile(logfile);
  qInstallMessageHandler([](QtMsgType type, const QMessageLogContext& ctx, const QString& msg) {
    omm::handle_log(logfile, level, print_long_message, type, ctx, msg);
  });

  auto options = std::make_unique<omm::Options>(false,  // is_cli
                                                true  // have_opengl
  );
  omm::Application app(qt_app, std::move(options));
  QApplication::setWindowIcon(omm::IconProvider::pixmap("omm"));

  omm::MainWindow window(app);
  app.set_main_window(window);
  window.show();

  if (argc > 1) {
    // https://stackoverflow.com/q/45718389/
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    app.scene.load_from(argv[1]);
  }

  app.scene.tool_box().set_active_tool(omm::SelectObjectsTool::TYPE);

  return QApplication::exec();
}
