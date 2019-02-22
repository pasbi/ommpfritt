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

std::vector<std::string> available_translations()
{
  std::list<std::string> trs;
  QDirIterator it(":", QDirIterator::Subdirectories);
  static const QString prefix(":/translation.");
  static const QString suffix(".qm");
  while (it.hasNext()) {
    const auto filename = it.next();
    if (filename.startsWith(prefix) && filename.endsWith(suffix)) {
      const int code_length = filename.size() - prefix.size()-  suffix.size();
      if (code_length < 0) {
        trs.push_back("");
      } else {
        const auto code = filename.mid(prefix.size(), code_length);
        trs.push_back(code.toStdString());
      }
    }
  }

  return std::vector(trs.begin(), trs.end());
}

bool load_translator(const QLocale& locale, QTranslator& translator)
{
  const auto locale_name = locale.name().toStdString();
  if (translator.load("translation." + locale.name(), ":", "_", ".qm")) {
    LOG(INFO) << "Installing translator for " << locale_name << ".";
    return true;
  } else {
    LOG(WARNING) << "No translator found for " << locale_name << ". Using fallback-translator.";
    LOG(INFO) << "Available locales: ";
    for (const auto& code : available_translations()) {
      LOG(INFO) << "    '" << code << "'";
    }
    if (translator.load("translation.qm", ":")) {
      LOG(INFO) << "Installing fallback-translator.";
    } else {
      LOG(ERROR) << "failed to load fallback-translator.";
    }
    return false;
  }
}

int main (int argc, char *argv[])
{
  google::InitGoogleLogging(argv[0]);


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

  QTranslator translator;
  const auto locale = QSettings().value("locale", QLocale()).toLocale();
  load_translator(locale, translator);
  qt_app.installTranslator(&translator);

  QCoreApplication::setOrganizationName(QObject::tr("omm"));
  QCoreApplication::setApplicationName(QObject::tr("ommpfritt"));

  omm::MainWindow window(app);
  app.set_main_window(window);
  window.show();

  if (argc > 1) {
    app.scene.load_from(argv[1]);
  }

  return qt_app.exec();
}
