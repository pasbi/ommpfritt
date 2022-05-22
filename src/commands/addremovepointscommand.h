#pragma once

#include "commands/command.h"
#include <deque>
#include <memory>

namespace omm
{

class Path;
class PathPoint;
class PathVector;
class PathObject;
class PathView;
class Edge;

class OwnedLocatedPath
{
public:
  explicit OwnedLocatedPath(Path* path, std::size_t point_offset, std::deque<std::unique_ptr<PathPoint>> points);
  ~OwnedLocatedPath();
  OwnedLocatedPath(OwnedLocatedPath&& other) = default;
  OwnedLocatedPath& operator=(OwnedLocatedPath&& other) = default;
  OwnedLocatedPath(const OwnedLocatedPath& other) = delete;
  OwnedLocatedPath& operator=(const OwnedLocatedPath& other) = delete;
  friend bool operator<(const OwnedLocatedPath& a, const OwnedLocatedPath& b);
  std::deque<std::unique_ptr<Edge>> create_edges();
  std::size_t point_offset() const;
  Path* path() const;

private:
  Path* m_path = nullptr;
  std::size_t m_point_offset;
  std::deque<std::unique_ptr<PathPoint>> m_points;
};


class AddRemovePointsCommand : public Command
{
public:
  class ChangeSet;

protected:
  explicit AddRemovePointsCommand(const QString& label, PathObject& path_object, std::deque<ChangeSet> changes);
  ~AddRemovePointsCommand() override;
  void restore_bridges();
  void restore_edges();
  std::deque<Edge*> owned_edges() const;

private:
  std::deque<ChangeSet> m_change_sets;
  PathObject& m_path_object;
  void update();
};

class AddPointsCommand : public AddRemovePointsCommand
{
public:
  explicit AddPointsCommand(PathObject& path_object, std::deque<OwnedLocatedPath> points_to_add);
  void undo() override;
  void redo() override;
  static QString static_label();
  std::deque<Edge*> new_edges() const;

private:
  const std::deque<Edge*> m_new_edges;
};

class RemovePointsCommand : public AddRemovePointsCommand
{
public:
  explicit RemovePointsCommand(PathObject& path_object, const std::deque<PathView>& points_to_remove);
  void undo() override;
  void redo() override;
  static QString static_label();
};

}  // namespace omm
