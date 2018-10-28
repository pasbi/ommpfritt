#pragma once

#include <QWidget>

namespace omm
{

class Project;

class Viewport : public QWidget
{
public:
  Viewport(Project& project, QWidget* parent = nullptr);
protected:
  void paintEvent(QPaintEvent* event);
};

}  // namespace omm