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
  explicit OwnedLocatedPath(Path* path, std::size_t point_offset, std::deque<std::shared_ptr<PathPoint>> points);
  ~OwnedLocatedPath();
  OwnedLocatedPath(OwnedLocatedPath&& other) = default;
  OwnedLocatedPath& operator=(OwnedLocatedPath&& other) = default;
  OwnedLocatedPath(const OwnedLocatedPath& other) = delete;
  OwnedLocatedPath& operator=(const OwnedLocatedPath& other) = delete;
  friend bool operator<(const OwnedLocatedPath& a, const OwnedLocatedPath& b);
  std::deque<std::unique_ptr<Edge>> create_edges() const;
  std::shared_ptr<PathPoint> single_point() const;
  std::size_t point_offset() const;
  Path* path() const;

private:
  Path* m_path = nullptr;
  std::size_t m_point_offset;
  std::deque<std::shared_ptr<PathPoint>> m_points;
};


class AddRemovePointsCommand : public Command
{
public:
  class ChangeSet;

  /**
   * @brief owned_edges the edges that this command owns.
   *  This changes when calling @code undo and @code redo, however, it is invariant when calling
   *  @code redo and @code undo subsequentially.
   */
  std::deque<Edge*> owned_edges() const;

protected:
  explicit AddRemovePointsCommand(const QString& label, ChangeSet changes, PathObject* path_object = nullptr);
  ~AddRemovePointsCommand() override;
  void restore_bridges();
  void restore_edges();

private:
  std::unique_ptr<ChangeSet> m_change_set;
  PathObject* m_path_object;
  void update();
};

class AddPointsCommand : public AddRemovePointsCommand
{
public:
  explicit AddPointsCommand(OwnedLocatedPath points_to_add, PathObject* path_object = nullptr);
  void undo() override;
  void redo() override;
  static QString static_label();
  /**
   * @brief new_edges the edges that are created when calling @code redo.
   *  After calling @code undo, this is the same as @code owned_edges.
   */
  std::deque<Edge*> new_edges() const;

private:
  const std::deque<Edge*> m_new_edges;
};

class RemovePointsCommand : public AddRemovePointsCommand
{
public:
  explicit RemovePointsCommand(const PathView& points_to_remove, PathObject* path_object = nullptr);
  void undo() override;
  void redo() override;
  static QString static_label();
};

}  // namespace omm
