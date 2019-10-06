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
  void contextMenuEvent(QContextMenuEvent *event) override;
  virtual void populate_menu(QMenu&);
  virtual std::vector<std::string> application_actions() const;
  bool event(QEvent* e) override;
  bool eventFilter(QObject* o, QEvent* e) override;

  /**
   * @brief child_key_press_event is called when a key event has occured on any child widget
   * @return return true if the event was handled by this function or false otherwise.
   * @note the default implementation does nothing and returns false.
   */
  virtual bool child_key_press_event(QWidget&, QKeyEvent&);

private:
  using QDockWidget::setWidget;  // use set_widget instead
};

void register_managers();

} // namespace omm

