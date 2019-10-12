#pragma once

#include "preferences/preferencestree.h"
#include "color/color.h"

namespace omm
{

class UiColors : public PreferencesTree
{
public:
  explicit UiColors();
  ~UiColors();
  QVariant data(int column, const PreferencesTreeValueItem &item, int role) const override;
  bool set_data(int column, PreferencesTreeValueItem& item, const QVariant &value) override;
  QPalette make_palette() const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};

}  // namespace omm
