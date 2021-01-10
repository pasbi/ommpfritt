#include "managers/curvemanager/curvemanagerquickaccessdelegate.h"
#include "animation/animator.h"
#include "logging.h"
#include "mainwindow/application.h"
#include "managers/curvemanager/curvetree.h"
#include "scene/scene.h"
#include <QMouseEvent>
#include <QPainter>

namespace
{
class VisibilityArea : public omm::QuickAccessDelegate::Area
{
public:
  VisibilityArea(omm::CurveTree& view, omm::Animator& animator, const QRectF& area);
  void draw(QPainter& painter, const QModelIndex& index) override;
  void begin(const QModelIndex& index, QMouseEvent& event) override;
  void perform(const QModelIndex& index, QMouseEvent& event) override;
  void end() override;

private:
  omm::CurveTree& m_view;
  omm::Animator& m_animator;
  bool m_visibility = false;
};

}  // namespace

namespace omm
{
CurveManagerQuickAccessDelegate::CurveManagerQuickAccessDelegate(Animator& animator,
                                                                 CurveTree& view)
    : QuickAccessDelegate(view)
{
  add_area(std::make_unique<VisibilityArea>(view, animator, QRectF(0.0, 0.0, 1.0, 1.0)));
}

}  // namespace omm

VisibilityArea::VisibilityArea(omm::CurveTree& view, omm::Animator& animator, const QRectF& area)
    : omm::QuickAccessDelegate::Area(area), m_view(view), m_animator(animator)
{
}

void VisibilityArea::draw(QPainter& painter, const QModelIndex& index)
{
  if (!index.isValid()) {
    return;
  }

  const QModelIndex sindex = m_view.map_to_source(index.siblingAtColumn(0));
  assert(!sindex.isValid() || sindex.model() == &m_animator);

  const auto visibility
      = m_animator.visit_item(sindex, [this](auto&& item) { return m_view.is_visible(item); });

  const auto color = [visibility]() {
    switch (visibility) {
    case omm::CurveTree::Visibility::Visible:
      return Qt::green;
    case omm::CurveTree::Visibility::Hidden:
      return Qt::red;
    case omm::CurveTree::Visibility::Undetermined:
      return Qt::yellow;
    default:
      Q_UNREACHABLE();
      return Qt::gray;
    }
  }();

  painter.fillRect(area, QColor(color));
}

void VisibilityArea::begin(const QModelIndex& index, QMouseEvent& event)
{
  is_active = true;
  const QModelIndex sindex = m_view.map_to_source(index.siblingAtColumn(0));
  if (sindex.isValid()) {
    if ((event.modifiers() & Qt::ControlModifier) != 0u) {
      is_active = false;
      m_view.hide_everything();
    }
    m_animator.visit_item(sindex, [this](auto&& item) {
      m_visibility = omm::CurveTree::Visibility::Visible != m_view.is_visible(item);
      m_view.set_visible(item, m_visibility);
    });
  }
}

void VisibilityArea::perform(const QModelIndex& index, [[maybe_unused]] QMouseEvent& event)
{
  const QModelIndex sindex = m_view.map_to_source(index.siblingAtColumn(0));
  if (sindex.isValid()) {
    m_animator.visit_item(sindex, [this](auto&& item) { m_view.set_visible(item, m_visibility); });
  }
}

void VisibilityArea::end()
{
  is_active = false;
}
