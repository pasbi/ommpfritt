#include <QAbstractItemDelegate>

namespace omm
{

class Animator;
class Track;

class TrackViewDelegate : public QAbstractItemDelegate
{
  Q_OBJECT
public:
  TrackViewDelegate(Animator& animator);
  void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
  QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

public Q_SLOTS:
  void remove_keyframe(Track& track, int frame);
  void insert_keyframe(Track& track, int frame);
  void move_keyframe(Track& track, int old_frame, int new_frame);
  void remove_track(Track& track);
  void insert_track(Track& track);
  void invalidate_cache();

private:
  Animator& m_animator;

  struct PersistentKeyFrame
  {
    bool is_selected = false;
  };

  std::map<Track*, std::map<int, PersistentKeyFrame>> m_keyframes;
  void draw_keyframe(QPainter& painter, const QRectF& rect, const PersistentKeyFrame& k) const;
};

}  // namespace omm
