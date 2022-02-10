#include "managers/stylemanager/stylelistview.h"
#include "main/application.h"
#include "renderers/style.h"
#include "scene/scene.h"
#include "scene/stylelist.h"
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPicture>
#include <QResizeEvent>
#include <QStyledItemDelegate>

namespace
{
constexpr int ICON_SIZE = 50;
}

namespace omm
{
StyleListView::StyleListView(StyleList& model)
    : ManagerItemView(model), m_icon_size(QSize(ICON_SIZE, ICON_SIZE)),
      m_item_delegate(*this, m_icon_size)
{
  setWrapping(true);
  setIconSize(m_icon_size);
  setResizeMode(QListView::Adjust);
  setFlow(QListView::LeftToRight);
  setSelectionRectVisible(true);
  setSpacing(4);

  // this actually mimics the effect that `setViewMode(QListView::IconMode)` should have.
  // Howver, the IconMode behaves strangely, the ListMode is much closer to what I want.
  // The style delegate simply styles the ListMode such that it looks like IconMode.
  setItemDelegate(&m_item_delegate);
  connect(&model, &StyleList::modelReset, this, &StyleListView::update_text_height);
  connect(&model, &StyleList::rowsInserted, this, &StyleListView::update_text_height);
}

void StyleListView::mouseReleaseEvent(QMouseEvent* e)
{
  this->model()->scene.set_selection(this->selected_items());
  ManagerItemView::mouseReleaseEvent(e);
}

void StyleListView::dropEvent(QDropEvent* e)
{
  ManagerItemView::dropEvent(e);
  update_layout();
}

void StyleListView::update_text_height()
{
  QPicture picture;
  QPainter painter(&picture);
  static constexpr auto large_number = 1000000;
  const QRect rectangle(QPoint(0, 0), QSize(m_icon_size.width(), large_number));
  // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDeleteLeaks)
  if (const auto* model = this->model(); model != nullptr) {
    const auto heights = util::transform(model->items(), [&painter, &rectangle](auto&& item) {
      QRect actual_rectangle;
      const QString text = item->name();
      painter.drawText(rectangle, text_flags, text, &actual_rectangle);
      return actual_rectangle.height();
    });
    if (!heights.empty()) {
      m_item_delegate.set_text_height(*std::max_element(heights.begin(), heights.end()));
      QTimer::singleShot(1, this, &StyleListView::update_layout);
    }
  }
}

void StyleListView::set_selection(const std::set<Style*>& selection)
{
  QItemSelection new_selection;
  for (Style* style : selection) {
    QModelIndex index = model()->index_of(*style);
    new_selection.merge(QItemSelection(index, index), QItemSelectionModel::Select);
  }
  selectionModel()->select(new_selection, QItemSelectionModel::ClearAndSelect);
  update_layout();
}

void StyleListView::update_layout()
{
  // force layout update
  QSize size = this->size();
  resize(size + QSize(1, 1));
  resize(size);
}

void StyleListView::mouseDoubleClickEvent(QMouseEvent* event)
{
  if (indexAt(event->pos()).isValid()) {
    ManagerItemView::mouseDoubleClickEvent(event);
  } else {
    Application::instance().perform_action("new style");
  }
}

void StyleListView::showEvent(QShowEvent* event)
{
  ManagerItemView::showEvent(event);
  update_layout();
}

}  // namespace omm
