#include "mainwindow/viewport/viewport.h"

#include <glog/logging.h>
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>

#include "renderers/viewportrenderer.h"
#include "scene/scene.h"

namespace
{

arma::vec2 point2vec(const QPoint& p)
{
  return arma::vec2 {
    static_cast<double>(p.x()),
    static_cast<double>(p.y()),
  };
}

void set_cursor_position(QWidget& widget, const arma::vec2& pos)
{
  auto cursor = widget.cursor();
  cursor.setPos(widget.mapToGlobal(QPoint(pos(0), pos(1))));
  widget.setCursor(cursor);
}

std::unique_ptr<omm::Handle> make_handle(const std::set<omm::HasProperties*>& selection)
{
  std::set<omm::Object*> objects;
  for (auto hp : selection) {
    if (hp->has_property<omm::ObjectTransformation>(omm::Object::TRANSFORMATION_PROPERTY_KEY)) {
      objects.insert(dynamic_cast<omm::Object*>(hp));
    }
  }
  return std::make_unique<omm::GlobalOrientedHandle>(objects);
}

// coordinate system of QWidget's canvas goes top-down and left-to-right
// I think bottom-up and left-to-right is more intuitive.
const auto TOP_RIGHT = omm::ObjectTransformation().scaled({1, -1});

}  // namespace

namespace omm
{

Viewport::Viewport(Scene& scene)
  : m_scene(scene)
  , m_timer(std::make_unique<QTimer>())
  , m_pan_controller([this](const arma::vec2& pos) { set_cursor_position(*this, pos); })
  , m_handle(make_handle(scene.selection()))
  , m_viewport_transformation(TOP_RIGHT)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  connect(&*m_timer, &QTimer::timeout, [this]() {
    update();
  });
  m_timer->setInterval(30);
  m_timer->start();

  m_scene.Observed<AbstractSelectionObserver>::register_observer(*this);
  setMouseTracking(true);
}

Viewport::~Viewport()
{
  m_scene.Observed<AbstractSelectionObserver>::unregister_observer(*this);
}

void Viewport::paintEvent(QPaintEvent* event)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.fillRect(rect(), Qt::gray);

  const auto width = static_cast<double>(this->width());
  const auto height = static_cast<double>(this->height());
  const BoundingBox render_box({ { 0, 0 },
                                 { width, height } });
  ViewportRenderer renderer(painter, render_box);

  renderer.set_base_transformation(viewport_transformation());
  renderer.render(m_scene);

  m_handle->draw(renderer);
}

void Viewport::mousePressEvent(QMouseEvent* event)
{
  const arma::vec2 cursor_position = point2vec(event->pos());
  m_pan_controller.init(cursor_position);

  if (event->modifiers() & Qt::AltModifier) {
    event->accept();
  }  else if (event->modifiers() == Qt::NoModifier) {
    if (m_handle->mouse_press(viewport_transformation().inverted().apply_to_position(cursor_position))) {
      event->accept();
    }
  } else {
    QWidget::mousePressEvent(event);
  }
}

void Viewport::mouseMoveEvent(QMouseEvent* event)
{
  const auto widget_size = arma::vec2{ static_cast<double>(width()),
                                       static_cast<double>(height()) };
  const auto cursor_position = point2vec(event->pos());
  arma::vec2 delta { 0.0, 0.0 };
  if (event->buttons() == Qt::LeftButton)
  {
    delta = m_pan_controller.delta(cursor_position, widget_size);
    if (event->modifiers() & Qt::AltModifier) {
      m_viewport_transformation.translate(delta);
      event->accept();
      return;
    }
  }

  if (event->modifiers() == Qt::NoModifier)
  {
    if (m_handle->mouse_move( viewport_transformation().inverted().apply_to_direction(delta),
                              viewport_transformation().inverted().apply_to_position(
                                cursor_position) ))
    {
      event->accept();
      return;
    }
  }

  QWidget::mouseMoveEvent(event);
}

void Viewport::mouseReleaseEvent(QMouseEvent* event)
{
  m_handle->mouse_release();
  QWidget::mouseReleaseEvent(event);
}

void Viewport::set_selection(const std::set<HasProperties*>& selection)
{
  m_handle = make_handle(selection);
}

ObjectTransformation Viewport::viewport_transformation() const
{
  const auto center = arma::vec2{ static_cast<double>(width()) / 2.0,
                                  static_cast<double>(height()) / 2.0 };
  return m_viewport_transformation.translated(center);
}

}  // namespace omm
