#pragma once

#include <memory>
#include <QWidget>
#include <QTimer>

#include "geometry/objecttransformation.h"

namespace omm
{

class Scene;

class MousePanArea : public QWidget
{
public:
  void init();
  arma::vec2 delta(arma::vec2 pos);
private:
  arma::vec2 m_last_position;
};

class Viewport : public MousePanArea
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
  arma::vec2 m_last_position;
};

}  // namespace omm
