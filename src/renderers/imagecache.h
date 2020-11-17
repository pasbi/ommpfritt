#pragma once

#include "cache.h"
#include <QImage>
#include <QPicture>

namespace omm
{
class ImageCache : public Cache<std::pair<QString, int>, QPicture>
{
protected:
  QPicture retrieve(const std::pair<QString, int>& key) const override;
};

}  // namespace omm
