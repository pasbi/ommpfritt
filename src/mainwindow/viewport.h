#pragma once

#include <memory>
#include <QWidget>
#include <QTimer>

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
  std::unique_ptr<QTimer> m_timer;
};

}  // namespace omm