#include "mainwindow/viewport/viewport.h"

#include "mainwindow/viewport/anchorhud.h"
#include "mainwindow/application.h"

#include <QPainter>
#include <QTimer>
#include <QMouseEvent>

#include "scene/scene.h"
#include "python/pythonengine.h"
#include "scene/messagebox.h"
#include "tools/toolbox.h"

namespace
{

void set_cursor_position(QWidget& widget, const omm::Vec2f& pos)
{
  auto cursor = widget.cursor();
  cursor.setPos(widget.mapToGlobal(pos.to_point()));
  widget.setCursor(cursor);
}

bool match_mouse_modifiers(const QMouseEvent& event, const QString& key)
{
  const auto mm = omm::preferences().mouse_modifiers.at(key);
  return mm.modifiers == event.modifiers();
}

bool match_mouse_button_and_modifiers(const QMouseEvent& event, const QString& key)
{
  const auto mm = omm::preferences().mouse_modifiers.at(key);
  return mm.button == event.button() && mm.modifiers == event.modifiers();
}

double discretize(double value)
{
  value = std::log10(std::max(0.00001, value));
  value -= std::fmod(value, 1.0);
  return std::pow(10.0, value);
}

template<typename VecT> VecT fold(const VecT& vec)
{
  return vec;
}

template<typename F, typename VecT, typename... VecTs> VecT fold(const VecT& vec1, const VecT& vec2, const VecTs&... tail)
{
  return fold<F>(F(vec1, vec2), tail...);
}

template<typename... VecTs> auto max(const VecTs&... vecs)
{
  using VecT = std::tuple_element_t<0, std::tuple<VecTs...>>;
  return fold<VecT::max>(vecs...);
}

}  // namespace

namespace omm
{

Viewport::Viewport(Scene& scene)
  : m_scene(scene)
  , m_timer(std::make_unique<QTimer>())
  , m_pan_controller([this](const Vec2f& pos) { set_cursor_position(*this, pos); })
  , m_renderer(scene, Painter::Category::Handles | Painter::Category::Objects)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setFocusPolicy(Qt::StrongFocus);

  setMouseTracking(true);
  connect(&scene.message_box(), SIGNAL(selection_changed(std::set<AbstractPropertyOwner*>)),
          this, SLOT(update()));

  connect(&scene.message_box(), SIGNAL(appearance_changed()), this, SLOT(update()));
  connect(&m_fps_limiter, &QTimer::timeout, [this]() {
    m_fps_brake = false;
    if (m_update_later) {
      update();
    }
  });

  m_headup_displays.push_back(std::make_unique<AnchorHUD>(*this));
}

void Viewport::draw_grid(QPainter &painter) const
{
  painter.resetTransform();
  const auto viewport_transformation = this->viewport_transformation();
  const auto viewport_transformation_i = viewport_transformation.inverted();
  painter.setTransform(viewport_transformation.to_qtransform());
  const auto extrema = {
    viewport_transformation_i.apply_to_position(Vec2f::o()),
    viewport_transformation_i.apply_to_position(Vec2f(0, height())),
    viewport_transformation_i.apply_to_position(Vec2f(width(), 0)),
    viewport_transformation_i.apply_to_position(Vec2f(width(), height()))
  };
  using minmax_t = Vec2f(*)(const Vec2f&, const Vec2f&);
  const auto maxf = static_cast<minmax_t>(Vec2f::max);
  const auto minf = static_cast<minmax_t>(Vec2f::min);
  const Vec2f min = std::accumulate(extrema.begin(), extrema.end(), *extrema.begin(), minf);
  const Vec2f max = std::accumulate(extrema.begin(), extrema.end(), *extrema.begin(), maxf);

  struct GridOptions
  {
    const double base;
    const QColor color;
    const double width;
  };

  for (const auto& options : { GridOptions { 100.0, QColor(0, 0, 0, 100), 0.5 },
                               GridOptions { 1000.0, QColor(0, 0, 0, 128), 1.0 },
                               GridOptions { 10000.0, QColor(0, 0, 0, 255), 2.0 } })
  {
    QPen pen;
    pen.setCosmetic(true);
    pen.setWidth(options.width);
    pen.setColor(options.color);
    painter.setPen(pen);
    const auto base_step = (max - min) / Vec2f(width(), height()) * options.base;
    for (std::size_t d : { 0, 1 }) {
      double step = discretize(base_step[d]);
      for (double t = min[d]; t <= max[d] + step; t += step) {
        const double tt = t - std::fmod(t, step);
        Vec2f a = min;
        Vec2f b = max;
        a[d] = tt;
        b[d] = tt;
        painter.drawLine(a.to_pointf(), b.to_pointf());
      }
    }
  }
}

#if USE_OPENGL
void Viewport::paintGL()
#else
void Viewport::paintEvent(QPaintEvent*)
#endif
{
  QPainter painter(this);
  painter.save();
  m_renderer.painter = &painter;

  painter.setRenderHint(QPainter::Antialiasing);
  painter.setRenderHint(QPainter::SmoothPixmapTransform);
  painter.fillRect(rect(), Qt::gray);

  const auto viewport_transformation = this->viewport_transformation();

  painter.save();
  draw_grid(painter);
  painter.restore();

  m_scene.object_tree().root().set_transformation(viewport_transformation);
  {
    QSignalBlocker blocker(&m_scene);
    m_scene.evaluate_tags();
  }
  m_renderer.render();

  auto& tool = m_scene.tool_box().active_tool();
  tool.viewport_transformation = viewport_transformation;
  tool.draw(m_renderer);
  m_renderer.painter = nullptr;
  painter.restore();

  for (const auto& hud : m_headup_displays) {
    painter.save();
    painter.resetTransform();
    painter.translate(hud->pos);
    painter.setClipRect(QRect(QPoint(), hud->size()));
    hud->draw(painter);
    painter.restore();
  }
}

void Viewport::mousePressEvent(QMouseEvent* event)
{
  const Vec2f cursor_pos = Vec2f(event->pos());

  const auto action = [event]() {
    if (match_mouse_button_and_modifiers(*event, "shift viewport")) {
      return MousePanController::Action::Pan;
    } else if (match_mouse_button_and_modifiers(*event, "zoom viewport")) {
      return MousePanController::Action::Zoom;
    } else {
      return MousePanController::Action::None;
    }
  }();

  m_pan_controller.start_move( viewport_transformation().inverted().apply_to_position(cursor_pos),
                               action);
  m_last_cursor_pos = cursor_pos;

  if (action != MousePanController::Action::None) {
    event->accept();
  } else if (auto* hud = find_headup_display(event->pos()); hud && hud->mouse_press(*event)) {
    event->accept();
  } else if (m_scene.tool_box().active_tool().mouse_press(cursor_pos, *event)) {
    event->accept();
  } else {
    ViewportBase::mousePressEvent(event);
    return;
  }
  update();
}

void Viewport::mouseMoveEvent(QMouseEvent* event)
{
  const auto cursor_pos = Vec2f(event->pos());
  const Vec2f delta = cursor_pos - m_last_cursor_pos;
  m_last_cursor_pos = cursor_pos;

  auto& tool = m_scene.tool_box().active_tool();
  if (tool.mouse_move(delta, cursor_pos, *event)) {
    event->accept();
  } else {
    bool accept = false;
    for (auto& hud : m_headup_displays) {
      accept |= hud->mouse_move(*event);
    }
    if (accept) {
      event->accept();
    } else {
      const bool panning = m_pan_controller.action() == MousePanController::Action::Pan
                           && match_mouse_modifiers(*event, "zoom viewport");
      const bool zooming = m_pan_controller.action() == MousePanController::Action::Zoom
                           && match_mouse_modifiers(*event, "shift viewport");
      if (panning || zooming) {
        m_pan_controller.apply(delta, m_viewport_transformation);
        event->accept();
      } else {
        ViewportBase::mouseMoveEvent(event);
      }
    }
  }
  update();
}

void Viewport::mouseReleaseEvent(QMouseEvent* event)
{
  if (!m_pan_controller.end_move()) {
    const auto cursor_pos = Vec2f(event->pos());
    m_scene.tool_box().active_tool().mouse_release(cursor_pos, *event);
    if (event->button() == Qt::RightButton) {
      auto menu = m_scene.tool_box().active_tool().make_context_menu(this);
      if (menu) {
        menu->exec(event->globalPos());
      }
    }
    ViewportBase::mouseReleaseEvent(event);
  }
  for (const auto& hud : m_headup_displays) {
    hud->mouse_release(*event);
  }
  ViewportBase::mouseReleaseEvent(event);
  update();
}

ObjectTransformation Viewport::viewport_transformation() const
{
  return m_viewport_transformation.translated(Vec2f(width(), height())/2.0);
}

Scene& Viewport::scene() const { return m_scene; }

void Viewport::reset()
{
  m_viewport_transformation = ObjectTransformation();
  update();
}

void Viewport::set_transformation(const ObjectTransformation& transformation)
{
  m_viewport_transformation = transformation;
  m_viewport_transformation.normalized();
}

void Viewport::keyPressEvent(QKeyEvent *event)
{
  if (!m_scene.tool_box().active_tool().key_press(*event)) {
    ViewportBase::keyPressEvent(event);
  }
}

void Viewport::resizeEvent(QResizeEvent* event)
{
  const int spacing = 10;
  const QPoint margin(10, 10);
  const QSize size = event->size();
  QPoint pos = QPoint(size.width(), size.height()) - margin;
  for (auto& hud : m_headup_displays) {
    const QSize size = hud->size();
    pos -= QPoint(0, size.height() + spacing);
    hud->pos = pos + QPoint(-size.width(), 0);
  }
  QWidget::resizeEvent(event);
}

void Viewport::update()
{
  static constexpr double fps = 30.0;
  if (!m_fps_brake) {
    m_fps_brake = true;
    m_update_later = false;
    ViewportBase::update();
    Q_EMIT updated();
    m_fps_limiter.start(static_cast<int>(1000.0/fps));
    m_fps_limiter.setSingleShot(true);
  } else {
    m_update_later = true;
  }
}

HeadUpDisplay* Viewport::find_headup_display(const QPoint& pos) const
{
  for (const auto& hud : m_headup_displays) {
    if (QRect(hud->pos, hud->size()).contains(pos)) {
      return hud.get();
    }
  }
  return nullptr;
}

}  // namespace omm
