#pragma once

#include <memory>
#include <QDockWidget>

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
  explicit Manager(const QString& title, Scene& scene);

  Scene& m_scene;
};

} // namespace omm

