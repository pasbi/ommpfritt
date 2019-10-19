#pragma once

#include <QDialog>
#include <memory>

namespace Ui { class NamedColorsDialog; }

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
  ~NamedColorsDialog();

public Q_SLOTS:
  void add();
  void remove();
  void setCurrent(const QModelIndex& index);

private:
  std::unique_ptr<Ui::NamedColorsDialog> m_ui;
  NamedColors& model() const;
  std::unique_ptr<NamedColorsHighlighProxyModel<QListView>> m_proxy;

};

}  // namespace omm
