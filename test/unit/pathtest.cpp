#include "commands/addremovepointscommand.h"
#include "geometry/point.h"
#include "gtest/gtest.h"
#include "path/path.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"
#include "path/pathview.h"


namespace
{

bool check_correspondence(const auto& a, const std::vector<std::size_t>& a_indices,
                          const auto& b, const std::vector<std::size_t>& b_indices)
{
  assert(a_indices.size() == b_indices.size());
  for (std::size_t i = 0; i < a_indices.size(); ++i) {
    if (a.at(a_indices.at(i)) != b.at(b_indices.at(i))) {
      return false;
    }
  }
  return true;
}

template<typename Status_>
class UndoRedoStackMock
{
public:
  using Status = Status_;
  struct StackItem
  {
    explicit StackItem(std::unique_ptr<omm::Command>&& command, Status&& before)
        : command(std::move(command))
        , before(std::move(before))
    {
    }

    std::unique_ptr<omm::Command> command;
    Status before;
    std::optional<Status> after;
  };

  const StackItem& submit(std::unique_ptr<omm::Command> command)
  {
    m_stack.erase(m_stack.begin() + m_index, m_stack.end());
    auto& current = m_stack.emplace_back(std::move(command), status());
    redo();
    current.after = status();
    return current;
  }

  virtual Status status() const = 0;

  void redo()
  {
    const auto& current = m_stack.at(m_index);
    ASSERT_EQ(current.before, status());
    current.command->redo();
    if (current.after.has_value()) {
      // this status does not yet exist if redo is called from submit.
      ASSERT_EQ(*current.after, status());
    }
    m_index += 1;
  }

  void undo()
  {
    m_index -= 1;
    const auto& current = m_stack.at(m_index);
    ASSERT_EQ(current.after, status());
    current.command->undo();
    ASSERT_EQ(current.before, status());
  }

  void undo_all()
  {
    while (m_index > 0) {
      ASSERT_NO_FATAL_FAILURE(undo());
    }
  }

  void redo_all()
  {
    while (m_index < m_stack.size()) {
      ASSERT_NO_FATAL_FAILURE(redo());
    }
  }

private:
  std::size_t m_index = 0;
  std::deque<StackItem> m_stack;
};

class PathAddPointsCommandUndoRedoStackMock : public UndoRedoStackMock<std::vector<omm::PathPoint*>>
{
public:
  explicit PathAddPointsCommandUndoRedoStackMock(const omm::Path& path)
      : m_path(path)
  {
  }

  Status status() const override
  {
    return m_path.points();
  }

private:
  const omm::Path& m_path;
};

class PathAddPointsCommandTest : public ::testing::Test
{
protected:
  explicit PathAddPointsCommandTest()
      : m_path(m_path_vector.add_path())
      , m_stack(m_path)
  {
  }

  void SetUp()
  {
    ASSERT_EQ(m_path_vector.paths().size(), 1);
    EXPECT_EQ(m_path_vector.paths().front(), &m_path);
    EXPECT_EQ(m_path.points().size(), 0);
  }

  const auto& submit_add_n_points_command(const std::size_t offset = 0, const std::size_t n = 1)
  {
    std::deque<std::shared_ptr<omm::PathPoint>> points;
    for (std::size_t i = 0; i < n; ++i) {
      points.emplace_back(std::make_unique<omm::PathPoint>(omm::Point{}, m_path.path_vector()));
    }
    omm::OwnedLocatedPath olp{&m_path, offset, points};
    std::deque<omm::OwnedLocatedPath> olps;
    olps.emplace_back(std::move(olp));
    return m_stack.submit(std::make_unique<omm::AddPointsCommand>(std::move(olps)));
  }

  const auto& submit_remove_single_point_front_command()
  {
    std::deque<omm::PathView> ptrs;
    ptrs.emplace_back(m_path, 0, 1);
    return m_stack.submit(std::make_unique<omm::RemovePointsCommand>(ptrs));
  }

  const auto& submit_add_edge_command()
  {
    std::deque<std::shared_ptr<omm::PathPoint>> points;
    points.emplace_back(std::make_unique<omm::PathPoint>(omm::Point{}, m_path.path_vector()));
    points.emplace_back(std::make_unique<omm::PathPoint>(omm::Point{}, m_path.path_vector()));
    omm::OwnedLocatedPath olp{&m_path, 0, points};
    std::deque<omm::OwnedLocatedPath> olps;
    olps.emplace_back(std::move(olp));
    return m_stack.submit(std::make_unique<omm::AddPointsCommand>(std::move(olps)));
  }

  omm::Path& path() const
  {
    return m_path;
  }

  bool has_distinct_points() const
  {
    const auto points = m_path.points();
    return std::set(points.begin(), points.end()).size() == points.size();
  }

  PathAddPointsCommandUndoRedoStackMock& stack()
  {
    return m_stack;
  }

private:
  omm::PathVector m_path_vector;
  omm::Path& m_path;
  PathAddPointsCommandUndoRedoStackMock m_stack;
};

}  // namespace

TEST_F(PathAddPointsCommandTest, AddSinglePoint)
{
  ASSERT_NO_FATAL_FAILURE(submit_add_n_points_command());
  ASSERT_EQ(path().points().size(), 1);
  ASSERT_NO_FATAL_FAILURE(stack().undo());
  ASSERT_TRUE(path().points().empty());
  ASSERT_NO_FATAL_FAILURE(stack().redo());
}

TEST_F(PathAddPointsCommandTest, RemoveSinglePoint)
{
  ASSERT_NO_FATAL_FAILURE(submit_add_n_points_command());
  const auto one_point = path().points();
  ASSERT_EQ(path().points().size() , 1);
  ASSERT_NO_FATAL_FAILURE(submit_remove_single_point_front_command());
  ASSERT_TRUE(path().points().empty());
  ASSERT_NO_FATAL_FAILURE(stack().undo());
  ASSERT_NO_FATAL_FAILURE(stack().redo());
}

TEST_F(PathAddPointsCommandTest, AddTwoPoints)
{
  ASSERT_NO_FATAL_FAILURE(submit_add_edge_command());
  ASSERT_TRUE(has_distinct_points());
  ASSERT_EQ(path().points().size(), 2);
  ASSERT_NO_FATAL_FAILURE(stack().undo());
  ASSERT_NO_FATAL_FAILURE(stack().redo());
}

TEST_F(PathAddPointsCommandTest, AddThreePointsFrontOneByOne)
{
  ASSERT_NO_FATAL_FAILURE(submit_add_n_points_command());
  ASSERT_EQ(path().points().size(), 1);

  ASSERT_NO_FATAL_FAILURE(submit_add_n_points_command());
  ASSERT_EQ(path().points().size(), 2);
  ASSERT_TRUE(has_distinct_points());

  ASSERT_NO_FATAL_FAILURE(submit_add_n_points_command());
  ASSERT_EQ(path().points().size(), 3);
  ASSERT_TRUE(has_distinct_points());

  ASSERT_NO_FATAL_FAILURE(stack().undo_all());
  ASSERT_TRUE(path().points().empty());
  ASSERT_NO_FATAL_FAILURE(stack().redo_all());
  ASSERT_EQ(path().points().size(), 3);
}

TEST_F(PathAddPointsCommandTest, AddPointsMiddle_A)
{
  ASSERT_NO_FATAL_FAILURE(submit_add_n_points_command(0, 4));
  ASSERT_TRUE(has_distinct_points());
  const auto four_points = path().points();
  ASSERT_EQ(four_points.size(), 4);

  ASSERT_NO_FATAL_FAILURE(submit_add_n_points_command(0, 1));
  ASSERT_TRUE(has_distinct_points());
  const auto five_points = path().points();
  ASSERT_EQ(five_points.size(), 5);
  ASSERT_TRUE(check_correspondence(five_points, {1, 2, 3, 4},
                                   four_points, {0, 1, 2, 3}));

  ASSERT_NO_FATAL_FAILURE(submit_add_n_points_command(1, 1););
  ASSERT_TRUE(has_distinct_points());
  const auto six_points = path().points();
  ASSERT_EQ(six_points.size(), 6);
  ASSERT_TRUE(check_correspondence(six_points, {0, 2, 3, 4, 5},
                                   five_points, {0, 1, 2, 3, 4}));

  ASSERT_NO_FATAL_FAILURE(submit_add_n_points_command(2, 3););
  ASSERT_TRUE(has_distinct_points());
  const auto nine_points = path().points();
  EXPECT_EQ(nine_points.size(), 9);
  ASSERT_TRUE(check_correspondence(nine_points, {0, 1, 5, 6, 7, 8},
                                   six_points,  {0, 1, 2, 3, 4, 5}));

  ASSERT_NO_FATAL_FAILURE(stack().undo_all());
  ASSERT_TRUE(path().points().empty());
  ASSERT_NO_FATAL_FAILURE(stack().redo_all());
  ASSERT_EQ(path().points().size(), 9);
}

TEST_F(PathAddPointsCommandTest, AddPointsMiddle_B)
{
  ASSERT_NO_FATAL_FAILURE(submit_add_n_points_command(0, 4));
  const auto four_points = path().points();
  ASSERT_EQ(four_points.size(), 4);

  const auto& b = submit_add_n_points_command(2, 3);
  const auto seven_points = path().points();
  ASSERT_EQ(seven_points.size(), 7);
  ASSERT_TRUE(check_correspondence(four_points, {0, 1, 2, 3}, seven_points, {0, 1, 5, 6}));
  ASSERT_EQ(dynamic_cast<omm::AddPointsCommand&>(*b.command).owned_edges().size(), 1);

  ASSERT_NO_FATAL_FAILURE(stack().undo());
  ASSERT_NO_FATAL_FAILURE(stack().redo());
}
