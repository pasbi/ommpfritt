#pragma once

#include <QMainWindow>

namespace omm
{

class Application;

class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  explicit MainWindow(Application& app);
};

}  // namespace omm