#include "managers/propertymanager/userpropertymanager/userpropertylistmodel.h"
#include "aspects/propertyowner.h"
#include <functional>

namespace
{

std::vector<omm::Property*> get_user_properties(const omm::AbstractPropertyOwner& property_owner)
{
  const auto predicate = [](const auto& property) { return property->is_user_property(); };
  const auto properties = property_owner.properties().values();
  return ::filter_if(properties, predicate);
}

}  // namespace

namespace omm
{

UserPropertyListModel::UserPropertyListModel(AbstractPropertyOwner& property_owner)
  : m_property_owner(property_owner)
{
}

int UserPropertyListModel::rowCount(const QModelIndex& index) const
{
  return get_user_properties(m_property_owner).size();
}

QVariant UserPropertyListModel::data(const QModelIndex& index, int role) const
{
  if (index.isValid()) {
    assert(!index.parent().isValid());
    if (role == Qt::DisplayRole) {
      const auto* property = get_user_properties(m_property_owner).at(index.row());
      return QString::fromStdString(property->label());
    }
  }
  return QVariant();
}

}  // namespace omm
