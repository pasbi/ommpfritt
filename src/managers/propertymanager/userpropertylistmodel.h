#pragma once

#include "properties/propertyconfiguration.h"
#include <QAbstractListModel>
#include <memory>

namespace omm
{
class AbstractPropertyOwner;
class Property;

class UserPropertyListItem
{
public:
  explicit UserPropertyListItem(Property* property = nullptr);
  [[nodiscard]] QString label() const;
  [[nodiscard]] QString type() const;

  PropertyConfiguration configuration;
  [[nodiscard]] Property* property() const
  {
    return m_property;
  }

private:
  Property* m_property = nullptr;
};

class UserPropertyListModel : public QAbstractListModel
{
  Q_OBJECT
public:
  explicit UserPropertyListModel(AbstractPropertyOwner& owner);
  [[nodiscard]] int rowCount(const QModelIndex& parent) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
  [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& parent) const override;
  UserPropertyListItem* item(const QModelIndex& index);
  bool setData(const QModelIndex& index, const QVariant& data, int role) override;
  [[nodiscard]] std::vector<const UserPropertyListItem*> items() const;
  bool contains(const Property* p) const;

public:
  void add_property(const QString& type);
  void del_property(const QModelIndex& index);

private:
  std::vector<UserPropertyListItem> m_items;
};

}  // namespace omm
