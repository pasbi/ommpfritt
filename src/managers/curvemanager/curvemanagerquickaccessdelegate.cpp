#include "managers/curvemanager/curvemanagerquickaccessdelegate.h"
#include "mainwindow/iconprovider.h"
#include "animation/animator.h"
#include "logging.h"
#include "main/application.h"
#include "managers/curvemanager/curvetreeview.h"
#include "scene/scene.h"
#include <QMouseEvent>
#include <QPainter>

namespace
{
class VisibilityArea : public omm::QuickAccessDelegate::Area
{
public:
  VisibilityArea(omm::CurveTreeView& view, omm::Animator& animator, const QRectF& area);
  void draw(QPainter& painter, const QModelIndex& index, const QRectF& rect) override;
  void begin(const QModelIndex& index, QMouseEvent& event) override;
  void perform(const QModelIndex& index, QMouseEvent& event) override;
  void end() override;

private:
  omm::CurveTreeView& m_view;
  omm::Animator& m_animator;
  bool m_visibility = false;
};

}  // namespace

namespace omm
{
CurveManagerQuickAccessDelegate::CurveManagerQuickAccessDelegate(Animator& animator,
                                                                 CurveTreeView& view)
    : QuickAccessDelegate(view)
{
  add_area(std::make_unique<VisibilityArea>(view, animator, QRectF(0.0, 0.0, 1.0, 1.0)));
}

}  // namespace omm

VisibilityArea::VisibilityArea(omm::CurveTreeView& view, omm::Animator& animator, const QRectF& area)
    : omm::QuickAccessDelegate::Area(area), m_view(view), m_animator(animator)
{
}

void VisibilityArea::draw(QPainter& painter, const QModelIndex& index, const QRectF& rect)
{
  if (!index.isValid()) {
    return;
  }

  const QModelIndex sindex = m_view.map_to_animator(index.siblingAtColumn(0));
  assert(!sindex.isValid() || sindex.model() == &m_animator);

  const auto visibility = m_animator.visit_item(sindex, [this](auto&& item) {
    return m_view.is_visible(item);
  });

  using namespace omm;

  auto icon = [visibility]() {
    switch (visibility) {
    case CurveTreeView::Visibility::Visible:
      return IconProvider::pixmap("visible");
    case CurveTreeView::Visibility::Hidden:
      return IconProvider::pixmap("invisible");
    case CurveTreeView::Visibility::Undetermined:
      return IconProvider::pixmap("partly-visible");
    default:
      Q_UNREACHABLE();
      return QPixmap{};
    }
  }();

  icon = icon.scaledToWidth(static_cast<int>(rect.width()), Qt::SmoothTransformation);
  const auto y = static_cast<int>(rect.center().y() - icon.height() / 2.0);
  painter.drawPixmap(static_cast<int>(rect.left()), y, icon);
}

void VisibilityArea::begin(const QModelIndex& index, QMouseEvent& event)
{
  is_active = true;
  const QModelIndex sindex = m_view.map_to_animator(index.siblingAtColumn(0));
  if (sindex.isValid()) {
    if ((event.modifiers() & Qt::ControlModifier) != 0u) {
      is_active = false;
      m_view.hide_everything();
    }
    m_animator.visit_item(sindex, [this](auto&& item) {
      m_visibility = omm::CurveTreeView::Visibility::Visible != m_view.is_visible(item);
      m_view.set_visible(item, m_visibility);
    });
  }
}

void VisibilityArea::perform(const QModelIndex& index, [[maybe_unused]] QMouseEvent& event)
{
  const QModelIndex sindex = m_view.map_to_animator(index.siblingAtColumn(0));
  if (sindex.isValid()) {
    m_animator.visit_item(sindex, [this](auto&& item) { m_view.set_visible(item, m_visibility); });
  }
}

void VisibilityArea::end()
{
  is_active = false;
}
