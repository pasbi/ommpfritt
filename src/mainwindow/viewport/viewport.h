#pragma once

#include <memory>
#include <QWidget>
#include <QTimer>

#include "geometry/objecttransformation.h"
#include "mainwindow/viewport/mousepancontroller.h"
#include "scene/abstractselectionobserver.h"
#include "mainwindow/viewport/handle.h"

namespace omm
{

class Scene;

class Viewport : public QWidget, public AbstractSelectionObserver
{
public:
  Viewport(Scene& scene);
  ~Viewport();
  Scene& scene() const;

protected:
  void paintEvent(QPaintEvent* event);
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void set_selection(const std::set<AbstractPropertyOwner*>& selection) override;

private:
  Scene& m_scene;
  std::unique_ptr<QTimer> m_timer;
  ObjectTransformation m_viewport_transformation;
  MousePanController m_pan_controller;
  std::unique_ptr<Handle> m_handle;
  ObjectTransformation viewport_transformation() const;
};

}  // namespace omm
