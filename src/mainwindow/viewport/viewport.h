#pragma once

#include <memory>
#include <QTimer>

#include "geometry/objecttransformation.h"
#include "mainwindow/viewport/mousepancontroller.h"
#include "scene/abstractselectionobserver.h"

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
  , public AbstractSelectionObserver
{
public:
  Viewport(Scene& scene);
  ~Viewport();
  Scene& scene() const;

protected:
#if USE_OPENGL
  void paintGL() override;
#else
  void paintEvent(QPaintEvent* event) override;
#endif
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void set_selection(const std::set<AbstractPropertyOwner*>& selection) override;

private:
  Scene& m_scene;
  std::unique_ptr<QTimer> m_timer;
  ObjectTransformation m_viewport_transformation;
  MousePanController m_pan_controller;
  ObjectTransformation viewport_transformation() const;
};

}  // namespace omm
