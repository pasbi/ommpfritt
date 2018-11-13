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

private:
  Scene& m_scene;
};

}  // namespace omm