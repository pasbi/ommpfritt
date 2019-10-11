#pragma once

#include "preferences/preferencestree.h"

namespace omm
{

class UiColors : public PreferencesTree
{
public:
  explicit UiColors();
  ~UiColors();
  QVariant data(const PreferencesTreeValueItem &value, int role) const override;
};

}  // namespace omm
