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

#include "mainwindow/mainwindow.h"

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
