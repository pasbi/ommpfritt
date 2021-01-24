#pragma once

#include <QDialog>
#include <memory>

namespace Ui
{
class NamedColorsDialog;
}

class QStyledItemDelegate;
class QListView;

namespace omm
{
template<typename ViewT> class NamedColorsHighlighProxyModel;
class NamedColors;

class NamedColorsDialog : public QDialog
{
  Q_OBJECT

public:
  explicit NamedColorsDialog(QWidget* parent = nullptr);
  ~NamedColorsDialog() override;
  NamedColorsDialog(NamedColorsDialog&&) = delete;
  NamedColorsDialog(const NamedColorsDialog&) = delete;
  NamedColorsDialog& operator=(NamedColorsDialog&&) = delete;
  NamedColorsDialog& operator=(const NamedColorsDialog&) = delete;

public:
  static void add();
  void remove();
  void setCurrent(const QModelIndex& index);

private:
  std::unique_ptr<Ui::NamedColorsDialog> m_ui;
  [[nodiscard]] static NamedColors& model();
  std::unique_ptr<NamedColorsHighlighProxyModel<QListView>> m_proxy;
  std::unique_ptr<QStyledItemDelegate> m_delegate;
};

}  // namespace omm
