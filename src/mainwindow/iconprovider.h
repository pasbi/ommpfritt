#pragma once

#include <QIcon>
#include "cachedgetter.h"

namespace omm
{

class AbstractPropertyOwner;

class IconProvider
{
public:
  explicit IconProvider() = default;

  QIcon icon(AbstractPropertyOwner& owner) const;
  QIcon icon(const QString& name) const;

private:
  mutable std::map<QString, QIcon> m_cached_icons_from_file;
  QIcon get_icon_by_filename(const QString& filename) const;
};

}  // namespace omm
