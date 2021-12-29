#pragma once

#include <QDialog>
#include <set>
#include <vector>

class QComboBox;

namespace omm
{

class PathObject;
class PointEdit;

class PointDialog : public QDialog
{
  Q_OBJECT
public:
  explicit PointDialog(const std::set<PathObject*>& paths, QWidget* parent = nullptr);
  ~PointDialog() override;
  PointDialog(PointDialog&&) = delete;
  PointDialog(const PointDialog&) = delete;
  PointDialog& operator=(PointDialog&&) = delete;
  PointDialog& operator=(const PointDialog&) = delete;

private:
  std::list<PointEdit*> m_point_edits;
  QComboBox* m_mode_combobox;
  static constexpr auto DISPLAY_MODE_SETTINGS_KEY = "PointDialog/display_mode";
};

}  // namespace omm
