#include "iconprovider.h"

QPixmap omm::IconProvider::get_icon(const std::string &icon_name, const QSize &size) const
{
  return get_icon_by_filename(":/icons/" + icon_name + ".png", size);
}

QPixmap omm::IconProvider::get_icon_by_filename(const std::string &filename, const QSize &size) const
{
  const IconKey key(filename, size);
  if (m_cached_icons.count(key) == 0) {
    const QIcon icon(QString::fromStdString(filename));
    m_cached_icons.insert(std::pair(key, icon.isNull() ? QPixmap() : icon.pixmap(size)));
  }
  return m_cached_icons[key];
}

omm::IconProvider::IconKey::IconKey(const std::string &filename, const QSize &size)
  : filename(filename), size(size) {}

bool omm::IconProvider::IconKey::operator<(const omm::IconProvider::IconKey &other) const
{
  if (other.filename == filename) {
    if (other.size.width() == size.width()) {
      return other.size.height() < size.height();
    } else {
      return other.size.width() < size.width();
    }
  } else {
    return other.filename < filename;
  }
}
