#pragma once

#include "common.h"
#include "managers/manageritemview.h"
#include "managers/stylemanager/stylelistviewitemdelegate.h"
#include "scene/stylelist.h"
#include <QListView>

namespace omm
{
class Object;
class StyleList;

class StyleListView : public ManagerItemView<QListView, StyleList>
{
  Q_OBJECT
public:
  explicit StyleListView(StyleList& model);
  using model_type = StyleList;
  using ManagerItemView<QListView, model_type>::ManagerItemView;
  void mouseReleaseEvent(QMouseEvent* e) override;
  void dropEvent(QDropEvent* e) override;
  void update_text_height();
  static constexpr int text_flags = Qt::AlignTop | Qt::AlignHCenter | Qt::TextWrapAnywhere;

protected:
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  void showEvent(QShowEvent* event) override;

public:
  void set_selection(const std::set<Style*>& selection);

private:
  const QSize m_icon_size;
  StyleListViewItemDelegate m_item_delegate;

private:
  void update_layout();
};

}  // namespace omm
