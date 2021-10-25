#pragma once

#include <set>

class QMouseEvent;
class QKeyEvent;
class QFocusEvent;

namespace omm
{

class AbstractPropertyOwner;

template<typename ItemViewT, typename ItemModelT> class ManagerItemView : public ItemViewT
{
public:
  using item_type = typename ItemModelT::structure_type::item_type;
  explicit ManagerItemView(ItemModelT& model);
  virtual ~ManagerItemView() = default;
  ManagerItemView(ManagerItemView&&) = delete;
  ManagerItemView(const ManagerItemView&) = delete;
  ManagerItemView& operator=(ManagerItemView&&) = delete;
  ManagerItemView& operator=(const ManagerItemView&) = delete;
  ItemModelT* model() const;
  [[nodiscard]] virtual std::set<AbstractPropertyOwner*> selected_items() const;

protected:
  void mousePressEvent(QMouseEvent* e) override;
  void keyPressEvent(QKeyEvent* e) override;
  void keyReleaseEvent(QKeyEvent* e) override;
  void focusInEvent(QFocusEvent* e) override;

private:
  using ItemViewT::setModel;
  bool m_block_selection_change_signal = false;
};

}  // namespace omm
