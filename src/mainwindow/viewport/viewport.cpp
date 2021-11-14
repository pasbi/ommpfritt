#include "mainwindow/viewport/viewport.h"
#include "mainwindow/iconprovider.h"

#include "main/application.h"
#include "mainwindow/viewport/anchorhud.h"

#include "preferences/uicolors.h"
#include "python/pythonengine.h"
#include "renderers/painter.h"
#include "renderers/painteroptions.h"
#include "scene/mailbox.h"
#include "scene/scene.h"
#include "tools/toolbox.h"
#include "tools/tool.h"
#include "tools/selectobjectstool.h"
#include "tools/selectpointstool.h"

#include <QMouseEvent>
#include <QPainter>
#include <QTimer>

namespace
{

using namespace omm;

void set_cursor_position(QWidget& widget, const omm::Vec2f& pos)
{
  auto cursor = widget.cursor();
  QCursor::setPos(widget.mapToGlobal(pos.to_point()));
  widget.setCursor(cursor);
}

double discretize(double value)
{
  static constexpr double eps = 0.00001;
  static constexpr double BASE_DEC = 10.0;
  value = std::log10(std::max(eps, value));
  value -= std::fmod(value, 1.0);
  return std::pow(BASE_DEC, value);
}

template<typename VecT> VecT fold(const VecT& vec)
{
  return vec;
}

template<typename F, typename VecT, typename... VecTs>
VecT fold(const VecT& vec1, const VecT& vec2, const VecTs&... tail)
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
    , m_pan_controller([this](const Vec2f& pos) { set_cursor_position(*this, pos); })
    , m_renderer(std::make_unique<Painter>(scene, Painter::Category::Handles | Painter::Category::Objects))
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setFocusPolicy(Qt::StrongFocus);

  setMouseTracking(true);
  connect(&scene.mail_box(), &MailBox::selection_changed, this, &Viewport::update);

  connect(&scene.mail_box(), &MailBox::scene_appearance_changed, this, &Viewport::update);
  connect(&m_fps_limiter, &QTimer::timeout, this, [this]() {
    m_fps_limiter.stop();
    if (m_update_later) {
      m_update_later = false;
      QWidget::update();
      Q_EMIT updated();
    }
  });

  m_headup_displays.push_back(std::make_unique<AnchorHUD>(*this));
  connect(&scene.tool_box(), &ToolBox::active_tool_changed, this, [this]() {
    update_cursor();
  });
}

Viewport::~Viewport() = default;

void Viewport::draw_grid(QPainter& painter,
                         const std::pair<Vec2f, Vec2f>& bounds,
                         Preferences::GridOption::ZOrder zorder) const
{
  const auto& [min, max] = bounds;
  painter.resetTransform();
  painter.setTransform(viewport_transformation().to_qtransform());
  for (const auto& [key, go] : preferences().grid_options) {
    if (go.zorder == zorder) {
      QPen pen;
      pen.setCosmetic(true);
      pen.setWidthF(go.pen_width);
      pen.setStyle(go.pen_style);
      pen.setColor(ui_color(*this, "Viewport", "grid " + key));

      painter.setPen(pen);
      const auto base_step = (max - min) / Vec2f(width(), height()) * go.base;
      for (std::size_t d : {0, 1}) {
        const double step = discretize(base_step[d]);
        const auto n = static_cast<std::size_t>((max[d] - min[d]) / step) + 1;
        for (std::size_t i = 0; i < n + 1; ++i) {
          const auto t = static_cast<double>(min[d] + static_cast<double>(i) * step);
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
}

std::pair<Vec2f, Vec2f> Viewport::compute_viewport_bounds() const
{
  const auto viewport_transformation_i = viewport_transformation().inverted();
  const auto extrema = {viewport_transformation_i.apply_to_position(Vec2f::o()),
                        viewport_transformation_i.apply_to_position(Vec2f(0, height())),
                        viewport_transformation_i.apply_to_position(Vec2f(width(), 0)),
                        viewport_transformation_i.apply_to_position(Vec2f(width(), height()))};
  using minmax_t = Vec2f (*)(const Vec2f&, const Vec2f&);
  const auto maxf = static_cast<minmax_t>(Vec2f::max);
  const auto minf = static_cast<minmax_t>(Vec2f::min);
  const Vec2f min = std::accumulate(extrema.begin(), extrema.end(), *extrema.begin(), minf);
  const Vec2f max = std::accumulate(extrema.begin(), extrema.end(), *extrema.begin(), maxf);
  return {min, max};
}

#if USE_OPENGL
void Viewport::paintGL()
#else
void Viewport::paintEvent(QPaintEvent*)
#endif
{
  QPainter painter(this);
  painter.save();
  m_renderer->painter = &painter;

  painter.setRenderHint(QPainter::Antialiasing);
  painter.setRenderHint(QPainter::SmoothPixmapTransform);
  painter.fillRect(rect(), ui_color(*this, "Viewport", "background"));

  const auto viewport_transformation = this->viewport_transformation();
  const auto viewport_bounds = compute_viewport_bounds();

  painter.save();
  draw_grid(painter, viewport_bounds, Preferences::GridOption::ZOrder::Background);
  painter.restore();

  m_scene.object_tree().root().set_transformation(viewport_transformation);

  PainterOptions options(*this);
  m_renderer->render(options);

  auto& tool = m_scene.tool_box().active_tool();
  tool.viewport_transformation = viewport_transformation;
  tool.draw(*m_renderer);
  m_renderer->painter = nullptr;
  painter.restore();

  painter.save();
  draw_grid(painter, viewport_bounds, Preferences::GridOption::ZOrder::Foreground);
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
    if (preferences().match("shift viewport", *event, true)) {
      return MousePanController::Action::Pan;
    } else if (preferences().match("zoom viewport", *event, true)) {
      return MousePanController::Action::Zoom;
    } else {
      return MousePanController::Action::None;
    }
  }();

  m_pan_controller.start_move(viewport_transformation().inverted().apply_to_position(cursor_pos),
                              action);
  m_last_cursor_pos = cursor_pos;

  if (action != MousePanController::Action::None) {
    event->accept();
  } else if (auto* hud = find_headup_display(event->pos());
             hud != nullptr && hud->mouse_press(*event)) {
    event->accept();
  } else if (m_scene.tool_box().active_tool().mouse_press(cursor_pos, *event)) {
    event->accept();
  } else {
    QWidget::mousePressEvent(event);
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
                           && preferences().match("shift viewport", *event, false);
      const bool zooming = m_pan_controller.action() == MousePanController::Action::Zoom
                           && preferences().match("zoom viewport", *event, false);
      if (panning || zooming) {
        m_pan_controller.apply(delta, m_viewport_transformation);
        event->accept();
      } else {
        QWidget::mouseMoveEvent(event);
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
    QWidget::mouseReleaseEvent(event);
  }
  for (const auto& hud : m_headup_displays) {
    hud->mouse_release(*event);
  }
  QWidget::mouseReleaseEvent(event);
  update();
}

ObjectTransformation Viewport::viewport_transformation() const
{
  return m_viewport_transformation.translated(Vec2f(width(), height()) / 2.0);
}

Scene& Viewport::scene() const
{
  return m_scene;
}

void Viewport::reset()
{
  m_viewport_transformation = ObjectTransformation();
  update();
}

void Viewport::set_transformation(const ObjectTransformation& transformation)
{
  m_viewport_transformation = transformation.normalized();
}

void Viewport::keyPressEvent(QKeyEvent* event)
{
  if (m_scene.tool_box().active_tool().key_press(*event)) {
    update();
  } else if (event->key() == Qt::Key_Escape) {
    m_scene.tool_box().activate_default_tool();
  } else {
    QWidget::keyPressEvent(event);
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
  static constexpr double SECOND_MS = 1000.0;
  if (m_fps_limiter.isActive()) {
    m_update_later = true;
  } else {
    QWidget::update();
    Q_EMIT updated();
    m_fps_limiter.start(static_cast<int>(SECOND_MS / fps));
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

void Viewport::update_cursor()
{
  const auto icon = [&tool=m_scene.tool_box().active_tool()]() {
    const auto icon = IconProvider::pixmap(tool.type() + "-cursor");
    if (icon.isNull()) {
      return IconProvider::pixmap("viewport-cursor");
    } else {
      return icon;
    }
  }();

  static constexpr QSize ICON_SIZE{32, 32};
  setCursor(QCursor{icon.scaled(ICON_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)});
}

}  // namespace omm
