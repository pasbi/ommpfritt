#pragma once

#include <memory>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QMenuBar>

#include "mainwindow/mainwindow.h"
#include "abstractfactory.h"
#include "keybindings/commandinterface.h"

namespace omm
{

class MainWindow;
class Scene;

class Manager
  : public QDockWidget
  , virtual public AbstractFactory<QString, Manager, Scene&>
  , virtual public CommandInterface
{
  Q_OBJECT   // Required for MainWindow::save_state
public:
  Manager(const Manager&) = delete;
  Manager(Manager&&) = delete;
  virtual ~Manager();
  Scene& scene() const;
  bool is_visible() const;
  bool is_locked() const { return m_is_locked; }

protected:
  Manager(const QString& title, Scene& scene);

  Scene& m_scene;
  void set_widget(std::unique_ptr<QWidget> widget);
  void keyPressEvent(QKeyEvent* e) override;


private:
  using QDockWidget::setWidget;  // use set_widget instead
  bool m_is_locked = false;

public Q_SLOTS:
  void set_locked(bool locked) { m_is_locked = locked; }

};

void register_managers();

} // namespace omm

