#pragma once

#include <QWidget>

namespace omm
{

class Scene;

class Viewport : public QWidget
{
public:
  Viewport(Scene& scene, QWidget* parent = nullptr);
protected:
  void paintEvent(QPaintEvent* event);
};

}  // namespace omm