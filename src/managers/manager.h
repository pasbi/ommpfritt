#pragma once

#include <unordered_set>
#include <memory>
#include <QDockWidget>

#include "mainwindow/mainwindow.h"

namespace omm
{

#define DECLARE_MANAGER_TYPE(classname) \
  public: \
    static constexpr auto TYPE() { return QT_TRANSLATE_NOOP("Manager", #classname); } \
    std::string type() const override { return TYPE(); }

class MainWindow;
class Scene;

class Manager : public QDockWidget
{
  Q_OBJECT
public:
  Manager(const Manager&) = delete;
  Manager(Manager&&) = delete;

  virtual std::string type() const = 0;

protected:
  explicit Manager(const QString& title, Scene& scene);


  Scene& m_scene;
};

} // namespace omm

