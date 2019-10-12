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
  std::string decode_data(const QVariant &value) const override;
  QVariant encode_data(const PreferencesTreeValueItem& item, int role) const override;
  QPalette make_palette() const;
  Color color(const QModelIndex& index) const;
  void set_color(const QModelIndex& index, const Color& color);
};

}  // namespace omm
