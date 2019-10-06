#pragma once

#include <memory>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QMenuBar>

#include "mainwindow/mainwindow.h"
#include "abstractfactory.h"

namespace omm
{

class MainWindow;
class Scene;

class Manager
  : public QDockWidget
  , virtual public AbstractFactory<std::string, Manager, Scene&>
  , virtual public CommandInterface
{
  Q_OBJECT   // Required for MainWindow::save_state
public:
  Manager(const Manager&) = delete;
  Manager(Manager&&) = delete;
  virtual ~Manager();
  Scene& scene() const;

protected:
  Manager(const QString& title, Scene& scene);

  Scene& m_scene;
  void set_widget(std::unique_ptr<QWidget> widget);
  void keyPressEvent(QKeyEvent* e) override;

private:
  using QDockWidget::setWidget;  // use set_widget instead
};

void register_managers();

} // namespace omm

