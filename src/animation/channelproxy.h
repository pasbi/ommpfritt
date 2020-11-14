#pragma once

#include <QObject>

namespace omm
{
class Track;

/**
 * @brief The Channel class' only purpose is to provide a pointer for `Animator`s
 * QAbstractItemModel-implementation.
 */
class ChannelProxy : public QObject
{
  Q_OBJECT
public:
  ChannelProxy(Track& track, std::size_t channel);

  Track& track;
  std::size_t channel;
};

}  // namespace omm
