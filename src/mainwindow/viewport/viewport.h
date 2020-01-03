#pragma once

#include <memory>
#include <QTimer>

#include "geometry/objecttransformation.h"
#include "mainwindow/viewport/mousepancontroller.h"
#include "renderers/painter.h"
#include "scene/scene.h"
#include "mainwindow/viewport/headupdisplay.h"
#include "preferences/preferences.h"

#define USE_OPENGL 0

#ifdef USE_OPENGL
#include <QWidget>
#else
#include <QOpenGLWidget>
#endif

namespace omm
{

class Scene;

#if USE_OPENGL
using ViewportBase = QOpenGLWidget;
#else
using ViewportBase = QWidget;
#endif

class Viewport : public ViewportBase
{
  Q_OBJECT
public:
  Viewport(Scene& scene);
  ~Viewport() = default;
  Scene& scene() const;
  void reset();
  void set_transformation(const ObjectTransformation& transformation);

  ObjectTransformation viewport_transformation() const;

protected:
#if USE_OPENGL
  void paintGL() override;
#else
  void paintEvent(QPaintEvent*) override;
#endif
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;

public Q_SLOTS:
  void update();

Q_SIGNALS:
  void updated();

private:
  Scene& m_scene;
  std::unique_ptr<QTimer> m_timer;
  ObjectTransformation m_viewport_transformation;
  MousePanController m_pan_controller;
  Painter m_renderer;
  Vec2f m_last_cursor_pos;

  QTimer m_fps_limiter;
  bool m_update_later = false;

  void draw_grid(QPainter& painter, const std::pair<Vec2f, Vec2f>& bounds,
                 Preferences::GridOption::ZOrder zorder) const;
  std::pair<Vec2f, Vec2f> compute_viewport_bounds() const;

  std::vector<std::unique_ptr<HeadUpDisplay>> m_headup_displays;
  HeadUpDisplay* find_headup_display(const QPoint& pos) const;
};

}  // namespace omm
