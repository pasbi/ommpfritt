#pragma once

#include <unordered_set>
#include <memory>
#include <QDockWidget>

#include "mainwindow/mainwindow.h"

namespace omm
{

class MainWindow;
class Scene;

class Manager : public QDockWidget
{
  Q_OBJECT
public:
  Manager(const Manager&) = delete;
  Manager(Manager&&) = delete;

protected:
  explicit Manager(const QString& title, Scene& scene);

  Scene& m_scene;
};

} // namespace omm

