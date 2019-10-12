#pragma once

#include "preferences/preferencestree.h"

namespace omm
{

class UiColors : public PreferencesTree
{
public:
  explicit UiColors();
  ~UiColors();
  QVariant data(const PreferencesTreeValueItem &value, int column, int role) const override;
  std::string set_data(const QVariant &value) const override;
};

}  // namespace omm
