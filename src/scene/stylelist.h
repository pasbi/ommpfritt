#pragma once

#include "scene/itemmodeladapter.h"
#include "scene/list.h"
#include <QAbstractListModel>

namespace omm
{
class Style;
class StyleList
    : public ItemModelAdapter<StyleList, Style, QAbstractListModel>
    , public List<Style>
{
  Q_OBJECT
public:
  explicit StyleList(Scene& scene);
  void insert(ListOwningContext<Style>& context) override;
  void remove(ListOwningContext<Style>& context) override;
  std::unique_ptr<Style> remove(Style& t) override;
  void move(ListMoveContext<Style>& context) override;
  std::deque<std::unique_ptr<Style>> set(std::deque<std::unique_ptr<Style>> items) override;

  [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role) override;
  [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
  [[nodiscard]] Style& item_at(const QModelIndex& index) const override;
  QModelIndex index_of(Style& style) const override;
  Scene& scene;
};

}  // namespace omm
