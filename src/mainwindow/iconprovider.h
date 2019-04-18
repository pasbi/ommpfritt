#pragma once

#include <QIcon>

namespace omm
{

class IconProvider
{
public:
  struct IconKey
  {
    explicit IconKey(const std::string& filename, const QSize& size);
    const std::string filename;
    const QSize size;
    bool operator<(const IconKey& other) const;
  };
  IconProvider() = default;

  QPixmap get_icon(const std::string& icon_name, const QSize& size) const;
  QPixmap get_icon_by_filename(const std::string& filename, const QSize& size) const;

private:
  mutable std::map<IconKey, QPixmap> m_cached_icons;
};

}  // namespace omm
