#pragma once

#include <QTimer>
#include <memory>

#include "geometry/objecttransformation.h"
#include "mainwindow/viewport/headupdisplay.h"
#include "mainwindow/viewport/mousepancontroller.h"
#include "preferences/preferences.h"
#include "renderers/painter.h"
#include "scene/scene.h"
#include <QWidget>

namespace omm
{
class Scene;

class Viewport : public QWidget
{
  Q_OBJECT
public:
  Viewport(Scene& scene);
  ~Viewport() override = default;
  Viewport(Viewport&&) = delete;
  Viewport(const Viewport&) = delete;
  Viewport& operator=(Viewport&&) = delete;
  Viewport& operator=(const Viewport&) = delete;

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

  void draw_grid(QPainter& painter,
                 const std::pair<Vec2f, Vec2f>& bounds,
                 Preferences::GridOption::ZOrder zorder) const;
  std::pair<Vec2f, Vec2f> compute_viewport_bounds() const;

  std::vector<std::unique_ptr<HeadUpDisplay>> m_headup_displays;
  HeadUpDisplay* find_headup_display(const QPoint& pos) const;
  void update_cursor();
};

}  // namespace omm
