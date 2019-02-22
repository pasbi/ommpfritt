#include <glog/logging.h>

#include "mainwindow/application.h"
#include "ommpfrittconfig.h"
#include "properties/property.h"
#include "propertywidgets/propertywidget.h"
#include "objects/object.h"
#include "managers/manager.h"
#include "tags/tag.h"
#include "tools/tool.h"
#include "serializers/abstractserializer.h"
#include "propertywidgets/propertyconfigwidget.h"
#include <QApplication>
#include <QTranslator>
#include "mainwindow/mainwindow.h"
#include <QSettings>
#include <QVariant>
#include <QDirIterator>

auto load_translator(const std::string& prefix, const QLocale& locale)
{
  auto translator = std::make_unique<QTranslator>();
  const auto locale_name = locale.name().toStdString();
  if (translator->load( QString("%1_%2").arg(prefix.c_str()).arg(locale.name()),
                        omm::MainWindow::LANGUAGE_RESOURCE_DIRECTORY, "_",
                        omm::MainWindow::LANGUAGE_RESOURCE_SUFFIX )) {
    LOG(INFO) << "Installing translator '" << prefix << "' for " << locale_name << ".";
    return translator;
  } else {
    LOG(WARNING) << "No translator '" << prefix << "' found for " << locale_name
                 << ". Using fallback-translator.";
    LOG(INFO) << "Available locales for '" << prefix << "': ";
    for (const auto& code : omm::MainWindow::available_translations()) {
      LOG(INFO) << "    '" << code << "'";
    }
    const auto fallback_tr_name = QString::fromStdString(prefix)
                                + omm::MainWindow::LANGUAGE_RESOURCE_SUFFIX;
    if (translator->load(fallback_tr_name, ":")) {
      LOG(INFO) << "Installing fallback-translator.";
      return translator;
    } else {
      LOG(ERROR) << "failed to load fallback-translator.";
      return std::unique_ptr<QTranslator>(nullptr);
    }
  }
}

auto install_translators(QCoreApplication& app, const QLocale& locale)
{
  const auto qms = { "qtbase", "omm" };
  const auto load_tr = [&app, locale](const std::string& qm) {
    auto translator = load_translator(qm, locale);
    if (translator) { app.installTranslator(translator.get()); };
    return translator;
  };
  return ::transform<std::unique_ptr<QTranslator>, std::vector>(qms, load_tr);
}

int main (int argc, char *argv[])
{
  google::InitGoogleLogging(argv[0]);

  QCoreApplication::setOrganizationName(QObject::tr("omm"));
  QCoreApplication::setApplicationName(QObject::tr("ommpfritt"));

  omm::register_properties();
  omm::register_managers();
  omm::register_propertywidgets();
  omm::register_objects();
  omm::register_tags();
  omm::register_serializers();
  omm::register_deserializers();
  omm::register_propertyconfigwidgets();
  omm::register_tools();

  QApplication qt_app(argc, argv);
  omm::Application app(qt_app);

  const auto locale = QSettings().value(omm::MainWindow::LOCALE_SETTINGS_KEY).toLocale();
  const auto translators = install_translators(qt_app, locale);

  omm::MainWindow window(app);
  app.set_main_window(window);
  window.show();

  if (argc > 1) {
    app.scene.load_from(argv[1]);
  }

  return qt_app.exec();
}
