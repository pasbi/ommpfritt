#pragma once

#include <QAbstractListModel>
#include <memory>

namespace omm
{

class AbstractPropertyOwner;
class UserPropertyListModel : public QAbstractListModel
{
public:
  explicit UserPropertyListModel(AbstractPropertyOwner& property_owner);
  int rowCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role) const override;

private:
  AbstractPropertyOwner& m_property_owner;

};

}  // namespace omm
