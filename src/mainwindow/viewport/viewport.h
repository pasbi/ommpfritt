#pragma once

#include <memory>
#include <QWidget>
#include <QTimer>

#include "geometry/objecttransformation.h"
#include "mainwindow/viewport/mousepancontroller.h"

namespace omm
{

class Scene;

class Viewport : public QWidget
{
public:
  Viewport(Scene& scene);

protected:
  void paintEvent(QPaintEvent* event);
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;

private:
  Scene& m_scene;
  std::unique_ptr<QTimer> m_timer;
  ObjectTransformation m_viewport_transformation;
  MousePanController m_pan_controller;
};

}  // namespace omm
