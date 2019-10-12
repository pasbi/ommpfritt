#pragma once

#include "preferences/preferencestree.h"

namespace omm
{

class UiColors : public PreferencesTree
{
public:
  explicit UiColors();
  ~UiColors();
  std::string decode_data(const QVariant &value) const override;
  QVariant encode_data(const PreferencesTreeValueItem& item, int role) const override;
};

}  // namespace omm
