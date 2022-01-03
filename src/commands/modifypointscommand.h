#pragma once

#include "commands/command.h"
#include <deque>
#include <memory>
#include "path/pathview.h"

namespace omm
{

class Path;
class PathPoint;
class PathObject;
class PathVector;
class Point;
struct PathView;

class ModifyPointsCommand : public Command
{
public:
  ModifyPointsCommand(const std::map<PathPoint*, Point>& points);
  void redo() override;
  void undo() override;
  [[nodiscard]] int id() const override;
  bool mergeWith(const QUndoCommand* command) override;
  [[nodiscard]] bool is_noop() const override;

private:
  std::map<PathPoint*, Point> m_data;
  void exchange();
};

class AbstractPointsCommand : public Command
{
public:
  class OwnedLocatedPath
  {
  public:
    explicit OwnedLocatedPath(Path* path, std::size_t index, std::deque<std::unique_ptr<PathPoint>>&& points);
    explicit OwnedLocatedPath(std::unique_ptr<Path> path);
    ~OwnedLocatedPath();
    OwnedLocatedPath(OwnedLocatedPath&& other) = default;
    OwnedLocatedPath& operator=(OwnedLocatedPath&& other) = default;
    OwnedLocatedPath(const OwnedLocatedPath& other) = delete;
    OwnedLocatedPath& operator=(const OwnedLocatedPath& other) = delete;
    PathView insert_into(PathVector& path_vector);
    friend bool operator<(const OwnedLocatedPath& a, const OwnedLocatedPath& b);

  private:
    Path* m_path = nullptr;
    std::unique_ptr<Path> m_owned_path{};
    std::size_t m_index;
    std::deque<std::unique_ptr<PathPoint>> m_points;
  };

protected:
  explicit AbstractPointsCommand(const QString& label,
                                 PathObject& path_object,
                                 std::deque<OwnedLocatedPath>&& points_to_add);
  explicit AbstractPointsCommand(const QString& label,
                                 PathObject& path_object,
                                 std::deque<PathView>&& points_to_remove);
  void add();
  void remove();

  [[nodiscard]] Scene& scene() const;
  ~AbstractPointsCommand() override;

public:
  AbstractPointsCommand(const AbstractPointsCommand&) = delete;
  AbstractPointsCommand(AbstractPointsCommand&&) = delete;
  AbstractPointsCommand& operator=(const AbstractPointsCommand&) = delete;
  AbstractPointsCommand& operator=(AbstractPointsCommand&&) = delete;

private:
  PathObject& m_path_object;
  std::deque<OwnedLocatedPath> m_points_to_add;
  std::deque<PathView> m_points_to_remove;
};

class AddPointsCommand : public AbstractPointsCommand
{
public:
  AddPointsCommand(PathObject& path_object, std::deque<OwnedLocatedPath>&& added_points);
  void redo() override;
  void undo() override;
  static QString static_label();
};

class RemovePointsCommand : public AbstractPointsCommand
{
public:
  RemovePointsCommand(PathObject& path_object, std::deque<PathView>&& removed_points);
  void redo() override;
  void undo() override;
};

}  // namespace omm
