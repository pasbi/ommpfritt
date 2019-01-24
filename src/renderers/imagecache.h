#pragma once

#include <QImage>

namespace omm
{

class ImageCache
{
public:
  QImage load(const std::string& filename);
  void clear();

private:
  std::map<std::string, QImage> m_cache;
};

}  // namespace
