#pragma once

#include <QAbstractListModel>
#include <memory>
#include "properties/property.h"

namespace omm
{

class AbstractPropertyOwner;

class UserPropertyListItem
{
public:
  explicit UserPropertyListItem(Property* property = nullptr);
  std::string label() const;
  std::string type() const;

  Property::Configuration configuration;
  Property* property() const { return m_property; }

private:
  Property* m_property;
};

class UserPropertyListModel : public QAbstractListModel
{
  Q_OBJECT
public:
  explicit UserPropertyListModel(AbstractPropertyOwner& owner);
  int rowCount(const QModelIndex& index) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  Qt::ItemFlags flags(const QModelIndex& parent) const override;
  UserPropertyListItem* item(const QModelIndex& index);
  bool setData(const QModelIndex& index, const QVariant& data, int role) override;
  std::vector<const UserPropertyListItem *> items() const;
  bool contains(const Property* p) const;

public Q_SLOTS:
  void add_property(const QString &type);
  void del_property(const QModelIndex& index);

private:
  std::vector<UserPropertyListItem> m_items;
};

}  // namespace omm
