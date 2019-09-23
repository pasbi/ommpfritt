#pragma once

#include <QTreeView>
#include <set>

namespace omm
{

class Animator;
class Track;

class DopeSheetView : public QTreeView
{
  Q_OBJECT;
public:
  explicit DopeSheetView(Animator& animator);

public Q_SLOTS:
  void update_second_column(Track& track);
  void update_second_column();

private:
  Animator& m_animator;
};

}  // namespace
