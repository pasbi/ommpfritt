#pragma once

#include <memory>
#include <QAbstractItemModel>
#include <QApplication>
#include <QAbstractItemView>
#include <QIdentityProxyModel>

namespace omm
{

template<typename BaseView>
class StyledItemView : public BaseView
{
private:
  class StyleProxyModel : public QIdentityProxyModel
  {
  public:
    explicit StyleProxyModel(QAbstractItemView& view) : m_view(view) {}
  private:
    QVariant data(const QModelIndex& index, int role) const
    {
      if (role == Qt::ForegroundRole) {
        const auto group = m_view.isEnabled() ? (m_view.window()->isActiveWindow()
                                                 ? QPalette::Active
                                                 : QPalette::Inactive)
                                              : QPalette::Disabled;
        const auto prole = m_view.selectionModel()->isSelected(index)
                            ? QPalette::HighlightedText
                            : QPalette::WindowText;
        return qApp->palette().color(group, prole);
      } else {
        return QIdentityProxyModel::data(index, role);
      }
    }

    QAbstractItemView& m_view;
  };
  std::unique_ptr<QIdentityProxyModel> m_proxy;

public:
  explicit StyledItemView(QWidget* parent = nullptr)
    : BaseView(parent), m_proxy(std::make_unique<StyleProxyModel>(*this))
  {}

  QAbstractItemModel* model() const { return m_proxy->sourceModel(); }

  void setModel(QAbstractItemModel* model)
  {
    m_proxy->setSourceModel(model);
    BaseView::setModel(m_proxy.get());
  }

};

}  // namespace omm
