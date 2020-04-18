#include "mainwindow/application.h"
#include "ommpfrittconfig.h"
#include "tools/selectobjectstool.h"
#include "tools/toolbox.h"
#include <QApplication>
#include <QTranslator>
#include "mainwindow/mainwindow.h"
#include <QSettings>
#include <QVariant>
#include <QDirIterator>
#include "logging.h"
#include "animation/track.h"
#include "registers.h"

auto load_translator(const QString& prefix, const QLocale& locale)
{
  auto translator = std::make_unique<QTranslator>();
  const auto locale_name = locale.name().toStdString();
  if (translator->load(prefix + "_" + locale.name(), ":/qm", "_", ".qm")) {
    LINFO << "Installing translator '" << prefix << "' for " << locale_name << ".";
    return translator;
  } else {
    LWARNING << "No translator '" << prefix << "' found for " << locale_name
             << ". Using fallback-translator.";
    if (translator->load(prefix + "_", ":/qm", "_", ".qm")) {
      LINFO << "Installing fallback-translator.";
      return translator;
    } else {
      LERROR << "failed to load fallback-translator.";
      return std::unique_ptr<QTranslator>(nullptr);
    }
  }
}

auto install_translators(QCoreApplication& app, const QLocale& locale)
{
  const auto qms = { "qtbase", "omm" };
  const auto load_tr = [&app, locale](const QString& qm) {
    auto translator = load_translator(qm, locale);
    if (translator) { app.installTranslator(translator.get()); };
    return translator;
  };
  return ::transform<std::unique_ptr<QTranslator>, std::vector>(qms, load_tr);
}

int main (int argc, char *argv[])
{
  QCoreApplication::setOrganizationName(QObject::tr("omm"));
  QCoreApplication::setApplicationName(QObject::tr("ommpfritt"));

  omm::register_everything();

  QApplication qt_app(argc, argv);

  const auto locale = QSettings().value(omm::MainWindow::LOCALE_SETTINGS_KEY).toLocale();
  const auto translators = install_translators(qt_app, locale);

  omm::Application app(qt_app);

  omm::MainWindow window(app);
  app.set_main_window(window);
  window.show();

  if (argc > 1) {
    app.scene.load_from(argv[1]);
  }

  app.scene.tool_box().set_active_tool(omm::SelectObjectsTool::TYPE);

  return qt_app.exec();
}
