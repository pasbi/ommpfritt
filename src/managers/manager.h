#pragma once

#include <memory>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QMenuBar>

#include "mainwindow/mainwindow.h"
#include "abstractfactory.h"

namespace omm
{

#define DECLARE_MANAGER_TYPE(classname) \
  public: \
    static constexpr auto TYPE() { return QT_TRANSLATE_NOOP("Manager", #classname); } \
    std::string type() const override { return TYPE(); }

class MainWindow;
class Scene;

class Manager : public QDockWidget, public AbstractFactory<std::string, Manager, Scene&>
{
  Q_OBJECT
public:
  Manager(const Manager&) = delete;
  Manager(Manager&&) = delete;
  virtual ~Manager() {}
  Scene& scene() const;

protected:
  explicit Manager( const QString& title, Scene& scene,
                    std::unique_ptr<QMenuBar> menu_bar = nullptr );

  Scene& m_scene;
  void set_widget(std::unique_ptr<QWidget> widget);

private:
  using QDockWidget::setWidget;  // use set_widget instead
};

void register_managers();

} // namespace omm

