#pragma once

#include "commands/command.h"
#include <deque>
#include <memory>

namespace omm
{

class Path;
class PathPoint;
class PathVector;
class PathView;
class Edge;

class OwnedLocatedPath
{
public:
  explicit OwnedLocatedPath(Path* path, std::size_t index, std::deque<std::unique_ptr<PathPoint>> points);
  ~OwnedLocatedPath();
  OwnedLocatedPath(OwnedLocatedPath&& other) = default;
  OwnedLocatedPath& operator=(OwnedLocatedPath&& other) = default;
  OwnedLocatedPath(const OwnedLocatedPath& other) = delete;
  OwnedLocatedPath& operator=(const OwnedLocatedPath& other) = delete;
  friend bool operator<(const OwnedLocatedPath& a, const OwnedLocatedPath& b);
  std::deque<std::unique_ptr<Edge>> create_edges();
  PathView path_view() const;

private:
  Path* m_path = nullptr;
  std::size_t m_index;
  std::deque<std::unique_ptr<PathPoint>> m_points;
};


class AddRemovePointsCommand : public Command
{
public:
  class ChangeSet;
protected:
  explicit AddRemovePointsCommand(const QString& label, std::deque<ChangeSet> changes);
  ~AddRemovePointsCommand() override;
  void restore_bridges();
  void restore_edges();

private:
  std::deque<ChangeSet> m_change_sets;
};

class AddPointsCommand : public AddRemovePointsCommand
{
public:
  explicit AddPointsCommand(std::deque<OwnedLocatedPath> points_to_add);
  void undo() override;
  void redo() override;
  static QString static_label();
};

class RemovePointsCommand : public AddRemovePointsCommand
{
public:
  explicit RemovePointsCommand(const std::deque<PathView>& points_to_remove);
  void undo() override;
  void redo() override;
  static QString static_label();
};

}  // namespace omm
