#pragma once

#include <memory>
#include <QTimer>

#include "geometry/objecttransformation.h"
#include "mainwindow/viewport/mousepancontroller.h"
#include "scene/abstractselectionobserver.h"
#include "renderers/viewportrenderer.h"

#define USE_OPENGL 0

#ifdef USE_OPENGL
#include <QWidget>
#else
#include <QOpenGLWidget>
#endif

namespace omm
{

class Scene;

class Viewport
#if USE_OPENGL
  : public QOpenGLWidget
#else
  : public QWidget
#endif
{
public:
  Viewport(Scene& scene);
  ~Viewport() = default;
  Scene& scene() const;
  void reset();

protected:
#if USE_OPENGL
  void paintGL() override;
#else
  void paintEvent(QPaintEvent* event) override;
#endif
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

private:
  Scene& m_scene;
  std::unique_ptr<QTimer> m_timer;
  ObjectTransformation m_viewport_transformation;
  MousePanController m_pan_controller;
  ViewportRenderer m_renderer;
  ObjectTransformation viewport_transformation() const;
};

}  // namespace omm
