#include "preferences/preferencestreeview.h"
#include "keybindings/keybindings.h"
#include "logging.h"
#include "preferences/preferencestreeviewdelegate.h"
#include <QHeaderView>

namespace omm
{
PreferencesTreeView::PreferencesTreeView(QWidget* parent) : QTreeView(parent)
{
  connect(this, &PreferencesTreeView::collapsed, this, &PreferencesTreeView::update_column_width);
  connect(this, &PreferencesTreeView::expanded, this, &PreferencesTreeView::update_column_width);
}

PreferencesTreeView::~PreferencesTreeView() = default;

void PreferencesTreeView::set_model(
    QAbstractItemModel& model,
    std::vector<std::unique_ptr<AbstractPreferencesTreeViewDelegate>> delegates)
{
  setModel(&model);
  m_column_delegates = std::move(delegates);
  for (std::size_t i = 0; i < m_column_delegates.size(); ++i) {
    m_column_delegates.at(i)->set_model(model);
    setItemDelegateForColumn(static_cast<int>(i) + 1, m_column_delegates.at(i).get());
  }
  setSelectionMode(QAbstractItemView::NoSelection);
  expandAll();
}

void PreferencesTreeView::transfer_editor_data_to_model()
{
  for (auto&& delegate : m_column_delegates) {
    delegate->transfer_editor_data_to_model();
  }
}

void PreferencesTreeView::close_all_editors()
{
  for (auto&& delegate : m_column_delegates) {
    delegate->close_current_editor();
  }
}

void PreferencesTreeView::resizeEvent(QResizeEvent* event)
{
  update_column_width();
  QTreeView::resizeEvent(event);
}

void PreferencesTreeView::update_column_width()
{
  const int required_width_name = sizeHintForColumn(0);
  const int required_width_others = [this]() {
    int width = 0;
    for (int i = 1; i < header()->count(); ++i) {
      width += sizeHintForColumn(i);
    }
    return width;
  }();
  const int available_width = viewport()->width();
  static constexpr int max_width_other = 350;

  const int excess = std::max(0, available_width - required_width_name - required_width_others);
  const int width_others
      = std::clamp(required_width_others + excess, required_width_others, max_width_other);
  const int width_name = std::max(available_width - width_others, required_width_name);

  setColumnWidth(0, width_name);
  const int n = header()->count();
  const double other_width = static_cast<double>(width_others) / static_cast<double>(n - 1);
  for (int i = 1; i < n; ++i) {
    setColumnWidth(i, static_cast<int>(other_width));
  }
}

}  // namespace omm
