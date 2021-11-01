#pragma once

#include <QAbstractItemDelegate>
#include <memory>

namespace omm
{
class QuickAccessDelegate : public QAbstractItemDelegate
{
public:
  explicit QuickAccessDelegate(QAbstractItemView& view);
  void paint(QPainter* painter,
             const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;
  [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem& option,
                               const QModelIndex& index) const override;

  bool on_mouse_button_press(QMouseEvent& event);
  void on_mouse_move(QMouseEvent& event);
  void on_mouse_release(QMouseEvent& event);

  class Area
  {
  public:
    explicit Area(const QRectF& area);
    virtual ~Area() = default;
    Area(Area&&) = delete;
    Area(const Area&) = delete;
    Area& operator=(Area&&) = delete;
    Area& operator=(const Area&) = delete;
    virtual void draw(QPainter& painter, const QModelIndex& index, const QRectF& rect) = 0;
    const QRectF area;
    bool is_active = false;
    virtual void begin(const QModelIndex& index, QMouseEvent& event) = 0;
    virtual void end() = 0;
    virtual void perform(const QModelIndex& index, QMouseEvent& event) = 0;
  };

protected:
  void add_area(std::unique_ptr<Area> area);

private:
  QAbstractItemView& m_view;
  std::list<std::unique_ptr<Area>> m_areas;
  [[nodiscard]] QPointF to_local(const QPoint& view_global, const QModelIndex& index) const;
};

}  // namespace omm
