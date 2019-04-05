#pragma once

#include <QDialog>
#include <set>

namespace omm
{

class Path;

class PointDialog : public QDialog
{
public:
  explicit PointDialog(const std::set<Path*>& paths, QWidget* parent = nullptr);
};

}  // namespace omm
