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
#include "logging.h"
#include "mainwindow/resourcemenu.h"
#include "animation/track.h"

auto load_translator(const std::string& prefix, const QLocale& locale)
{
  auto translator = std::make_unique<QTranslator>();
  const auto locale_name = locale.name().toStdString();
  if (translator->load( QString("%1_%2").arg(prefix.c_str()).arg(locale.name()),
                        omm::LanguagePlugin::RESOURCE_DIRECTORY, "_",
                        omm::LanguagePlugin::RESOURCE_SUFFIX )) {
    LINFO << "Installing translator '" << prefix << "' for " << locale_name << ".";
    return translator;
  } else {
    LWARNING << "No translator '" << prefix << "' found for " << locale_name
                 << ". Using fallback-translator.";
    LINFO << "Available locales for '" << prefix << "': ";
    for (const auto& code : omm::LanguageMenu::available_keys()) {
      LINFO << "    '" << code << "'";
    }
    const auto fallback_tr_name = QString::fromStdString(prefix)
                                + omm::LanguagePlugin::RESOURCE_SUFFIX;
    if (translator->load(fallback_tr_name, ":")) {
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
  const auto load_tr = [&app, locale](const std::string& qm) {
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

  const auto locale = QSettings().value(omm::MainWindow::LOCALE_SETTINGS_KEY).toLocale();
  const auto translators = install_translators(qt_app, locale);

  omm::Application app(qt_app);

  const QString skin = QSettings().value(omm::MainWindow::SKIN_SETTINGS_KEY).toString();
  if (!skin.isEmpty()) {
    LINFO << "load skin " << skin;
    omm::SkinPlugin::load_skin(skin);
  }

  omm::MainWindow window(app);
  app.set_main_window(window);
  window.show();

  if (argc > 1) {
    app.scene.load_from(argv[1]);
  }

  return qt_app.exec();
}
