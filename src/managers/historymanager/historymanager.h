#pragma once

#include "managers/manager.h"

class QListView;

namespace omm
{

class HistoryModel;

class HistoryManager : public Manager
{
  Q_OBJECT
public:
  HistoryManager(Scene& scene);

  static constexpr auto TYPE = QT_TRANSLATE_NOOP("any-context", "HistoryManager");
  std::string type() const override;

private:
  QListView* m_view;
  HistoryModel& m_model;
};

}  // namespace omm
