#include <glog/logging.h>

#include "mainwindow/application.h"
#include "ommpfrittconfig.h"
#include "properties/property.h"
#include "propertywidgets/propertywidget.h"
#include "objects/object.h"
#include "tags/tag.h"
#include "serializers/abstractserializer.h"
#include <QApplication>

#include "mainwindow/mainwindow.h"

int main (int argc, char *argv[])
{
  omm::Property::register_properties();
  omm::AbstractPropertyWidget::register_propertywidgets();
  omm::Object::register_objects();
  omm::Tag::register_tags();
  omm::AbstractSerializer::register_serializers();
  omm::AbstractDeserializer::register_deserializers();

  google::InitGoogleLogging(argv[0]);
  QApplication qt_app(argc, argv);
  omm::Application app(qt_app);

  QCoreApplication::setOrganizationName("omm");
  QCoreApplication::setApplicationName("ommpfritt");

  omm::MainWindow window(app);
  app.set_main_window(window);
  window.show();
  return qt_app.exec();
}