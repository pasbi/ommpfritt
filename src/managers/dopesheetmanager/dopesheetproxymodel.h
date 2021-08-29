#pragma once

#include <QIdentityProxyModel>

namespace omm
{

class Animator;

class DopeSheetProxyModel : public QIdentityProxyModel
{
public:
  explicit DopeSheetProxyModel(Animator& animator);
  [[nodiscard]] int rowCount(const QModelIndex& index) const override;
  [[nodiscard]] bool hasChildren(const QModelIndex& index) const override;
  [[nodiscard]] int columnCount(const QModelIndex&) const override;
  [[nodiscard]] QVariant data(const QModelIndex& proxyIndex, int role) const override;

private:
  [[nodiscard]] Animator& animator() const;
  Animator& m_animator;
};

}  // namespace omm
