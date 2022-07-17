#include "commands/addremovepointscommand.h"
#include "geometry/point.h"
#include "gtest/gtest.h"
#include "path/edge.h"
#include "path/path.h"
#include "path/pathpoint.h"
#include "path/pathvector.h"
#include "path/pathview.h"

#include <fmt/format.h>


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

class PathCommandUndoRedoStackMock : public UndoRedoStackMock<std::vector<omm::PathPoint*>>
{
public:
  explicit PathCommandUndoRedoStackMock(const omm::Path& path)
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

class PathCommandTest : public ::testing::Test
{
protected:
  explicit PathCommandTest()
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
    return m_stack.submit(std::make_unique<omm::AddPointsCommand>(omm::OwnedLocatedPath{&m_path, offset, points}));
  }

  const auto& submit_remove_point_command(const std::size_t offset = 0, const std::size_t n = 1)
  {
    return m_stack.submit(std::make_unique<omm::RemovePointsCommand>(omm::PathView(m_path, offset, n)));
  }

  const auto& submit_add_edge_command()
  {
    std::deque<std::shared_ptr<omm::PathPoint>> points;
    points.emplace_back(std::make_unique<omm::PathPoint>(omm::Point{}, m_path.path_vector()));
    points.emplace_back(std::make_unique<omm::PathPoint>(omm::Point{}, m_path.path_vector()));
    return m_stack.submit(std::make_unique<omm::AddPointsCommand>(omm::OwnedLocatedPath(&m_path, 0, points)));
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

  PathCommandUndoRedoStackMock& stack()
  {
    return m_stack;
  }

private:
  omm::PathVector m_path_vector;
  omm::Path& m_path;
  PathCommandUndoRedoStackMock m_stack;
};

struct RemoveAddPointsCommandTestParameter
{
  using Info = ::testing::TestParamInfo<RemoveAddPointsCommandTestParameter>;
  const std::size_t initial_point_count;
  const std::size_t offset;
  const std::size_t count;
protected:
  static std::string name_generator(const Info& info, std::string_view what);
};

struct RemovePointsCommandTestParameter : RemoveAddPointsCommandTestParameter
{
  static std::string name_generator(const Info& info)
  {
    return RemoveAddPointsCommandTestParameter::name_generator(info, "remove");
  }
};

struct AddPointsCommandTestParameter : RemoveAddPointsCommandTestParameter
{
  static std::string name_generator(const Info& info)
  {
    return RemoveAddPointsCommandTestParameter::name_generator(info, "add");
  }
};

class RemovePointsCommandTest
    : public PathCommandTest
    , public ::testing::WithParamInterface<RemoveAddPointsCommandTestParameter>
{
};

std::string RemoveAddPointsCommandTestParameter::name_generator(const Info& info, const std::string_view what)
{
  const auto begin = info.param.offset;
  const auto end = begin + info.param.count - 1;
  const auto n = info.param.initial_point_count;
  return fmt::format("{}_points_{}_to_{}_from_{}_point_path", what, begin, end, n);
}

class AddPointsCommandTest
    : public PathCommandTest
    , public ::testing::WithParamInterface<RemoveAddPointsCommandTestParameter>
{
};

class Range
{
public:
  Range(std::size_t begin, std::size_t end, const std::vector<Range>& ranges = {})
      : begin(begin), end(end), ranges(ranges)
  {
  }

  operator std::vector<std::size_t>() const
  {
    std::list<std::size_t> items;
    for (std::size_t i = begin; i < end; ++i) {
      items.push_back(i);
    }
    for (const auto& r : ranges) {
      const auto vs = static_cast<std::vector<std::size_t>>(r);
      items.insert(items.end(), vs.begin(), vs.end());
    }
    return std::vector(items.begin(), items.end());
  }

  friend Range operator+(const Range& a, const Range& b)
  {
    return Range{0, 0, {a, b}};
  }

  const std::size_t begin;
  const std::size_t end;
  std::vector<Range> ranges;
};

}  // namespace

TEST_F(PathCommandTest, AddSinglePoint)
{
  ASSERT_NO_FATAL_FAILURE(submit_add_n_points_command());
  ASSERT_EQ(path().points().size(), 1);
  ASSERT_NO_FATAL_FAILURE(stack().undo());
  ASSERT_TRUE(path().points().empty());
  ASSERT_NO_FATAL_FAILURE(stack().redo());
}

TEST_F(PathCommandTest, AddTwoPoints)
{
  ASSERT_NO_FATAL_FAILURE(submit_add_edge_command());
  ASSERT_TRUE(has_distinct_points());
  ASSERT_EQ(path().points().size(), 2);
  ASSERT_NO_FATAL_FAILURE(stack().undo());
  ASSERT_NO_FATAL_FAILURE(stack().redo());
}

TEST_F(PathCommandTest, AddThreePointsFrontOneByOne)
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

TEST_F(PathCommandTest, AddPointsMiddle_A)
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

TEST_F(PathCommandTest, AddPointsMiddle_B)
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

TEST_P(RemovePointsCommandTest, RemovePoints)
{
  const auto p = GetParam();
  ASSERT_NO_FATAL_FAILURE(submit_add_n_points_command(0, p.initial_point_count));
  const auto initial_points = path().points();
  ASSERT_EQ(initial_points.size(), p.initial_point_count);
  ASSERT_NO_FATAL_FAILURE(submit_remove_point_command(p.offset, p.count));
  const auto final_points = path().points();
  ASSERT_EQ(final_points.size(), p.initial_point_count - p.count);
  ASSERT_TRUE(check_correspondence(initial_points,
                                   Range(0, p.offset) + Range(p.offset + p.count, p.initial_point_count),
                                   final_points,
                                   Range(0, final_points.size())));
  ASSERT_NO_FATAL_FAILURE(stack().undo());
  ASSERT_NO_FATAL_FAILURE(stack().redo());
}

INSTANTIATE_TEST_SUITE_P(X, RemovePointsCommandTest,
                         ::testing::ValuesIn(std::vector<RemoveAddPointsCommandTestParameter>{
                                              {.initial_point_count = 1, .offset = 0, .count = 1},
                                              {.initial_point_count = 2, .offset = 0, .count = 1},
                                              {.initial_point_count = 2, .offset = 1, .count = 1},
                                              {.initial_point_count = 3, .offset = 0, .count = 1},
                                              {.initial_point_count = 3, .offset = 1, .count = 1},
                                              {.initial_point_count = 3, .offset = 2, .count = 1},
                                              {.initial_point_count = 2, .offset = 0, .count = 2},
                                              {.initial_point_count = 3, .offset = 0, .count = 2},
                                              {.initial_point_count = 3, .offset = 1, .count = 2},
                                              {.initial_point_count = 5, .offset = 0, .count = 5},
                                              {.initial_point_count = 5, .offset = 0, .count = 2},
                                              {.initial_point_count = 5, .offset = 1, .count = 2},
                                              {.initial_point_count = 5, .offset = 3, .count = 2}
                                                     }),
                          &RemovePointsCommandTestParameter::name_generator);

TEST_P(AddPointsCommandTest, AddPoints)
{
  const auto p = GetParam();
  if (p.initial_point_count > 0) {
    ASSERT_NO_FATAL_FAILURE(submit_add_n_points_command(0, p.initial_point_count));
  }
  const auto initial_points = path().points();
  ASSERT_EQ(initial_points.size(), p.initial_point_count);
  LINFO << "before: " << path().print_edge_info();
  ASSERT_NO_FATAL_FAILURE(submit_add_n_points_command(p.offset, p.count));
  const auto final_points = path().points();
  ASSERT_EQ(final_points.size(), p.initial_point_count + p.count);
  LINFO << "after: " << path().print_edge_info();
  ASSERT_TRUE(check_correspondence(initial_points, Range(0, p.initial_point_count),
                                   final_points, Range(0, p.offset) + Range(p.offset + p.count, final_points.size())));
  ASSERT_NO_FATAL_FAILURE(stack().undo());
  ASSERT_NO_FATAL_FAILURE(stack().redo());
}

INSTANTIATE_TEST_SUITE_P(X, AddPointsCommandTest,
                         ::testing::ValuesIn(std::vector<RemoveAddPointsCommandTestParameter>{
                                              {.initial_point_count = 0, .offset = 0, .count = 1},
                                              {.initial_point_count = 0, .offset = 0, .count = 2},
                                              {.initial_point_count = 0, .offset = 0, .count = 3},
                                              {.initial_point_count = 2, .offset = 0, .count = 1},
                                              {.initial_point_count = 2, .offset = 1, .count = 1},
                                              {.initial_point_count = 2, .offset = 2, .count = 1},
                                              {.initial_point_count = 2, .offset = 0, .count = 2},
                                              {.initial_point_count = 2, .offset = 1, .count = 2},
                                              {.initial_point_count = 2, .offset = 2, .count = 2},
                                              {.initial_point_count = 2, .offset = 0, .count = 3},
                                              {.initial_point_count = 2, .offset = 1, .count = 3},
                                              {.initial_point_count = 2, .offset = 2, .count = 3},
                                              {.initial_point_count = 3, .offset = 0, .count = 2},
                                              {.initial_point_count = 3, .offset = 1, .count = 2},
                                              {.initial_point_count = 3, .offset = 2, .count = 2},
                                              {.initial_point_count = 3, .offset = 3, .count = 2},
                                            }),
                          &AddPointsCommandTestParameter::name_generator);

class PathVectorCopy : public ::testing::Test
{
public:
  explicit PathVectorCopy()
  {
  }

  omm::PathVector& make_copy()
  {
    pv_copy = pv_original;
    return pv_copy;
  }

  void assert_valid() const
  {
    ASSERT_NO_FATAL_FAILURE(assert_valid_tangents(pv_copy));
    ASSERT_NO_FATAL_FAILURE(assert_valid_tangents(pv_original));
    ASSERT_NO_FATAL_FAILURE(assert_equiv_but_distinct());
  }

private:
  static void assert_valid_tangents(const omm::PathVector& path_vector)
  {
    for (const auto* path : path_vector.paths()) {
      for (const auto* const p : path->points()) {
        for (const auto& [key, tangent] : p->geometry().tangents()) {
//          ASSERT_TRUE(key.path == path || key.path == nullptr) << "key.path: " << key.path << ", path: " << path;
        }
      }
    }
  }

  static void assert_equiv_but_distinct(const omm::Path& a, const omm::Path& b)
  {
    ASSERT_NE(&a, &b) << "Identical paths expected to be distinguishable.";
    const auto ps_a = a.points();
    const auto ps_b = b.points();
    ASSERT_EQ(ps_a.size(), ps_b.size());
    for (std::size_t i = 0; i < ps_a.size(); ++i) {
      const auto& a = *ps_a.at(i);
      const auto& b = *ps_b.at(i);
      ASSERT_NE(&a, &b) << "Identical path points expected to be distinguishable.";
      ASSERT_NE(&a.geometry(), &b.geometry()) << "Geometry shall not be shared among path points.";
      ASSERT_EQ(a.geometry().position(), b.geometry().position());
      ASSERT_EQ(a.geometry().tangents().size(), b.geometry().tangents().size());
    }
  }

  void assert_equiv_but_distinct() const
  {
    const auto a_paths = pv_original.paths();
    const auto b_paths = pv_copy.paths();
    ASSERT_EQ(a_paths.size(), b_paths.size());
    for (std::size_t i = 0; i < a_paths.size(); ++i) {
      ASSERT_NO_FATAL_FAILURE(assert_equiv_but_distinct(*a_paths.at(i), *b_paths.at(i)));
    }
  }

protected:
  omm::PathVector pv_original;
  omm::PathVector pv_copy;
};

TEST_F(PathVectorCopy, OE)
{
  auto pv1_p0 = std::make_shared<omm::PathPoint>(omm::Point({0, 0}), &pv_original);
  auto pv1_p1 = std::make_shared<omm::PathPoint>(omm::Point({0, 1}), &pv_original);
  auto& pv1_path = pv_original.add_path();
  pv1_path.add_edge(std::make_unique<omm::Edge>(pv1_p0, pv1_p1, &pv1_path));
  pv1_p0->geometry().set_tangent({&pv1_path, omm::Point::Direction::Backward}, omm::PolarCoordinates());
  pv1_p1->geometry().set_tangent({&pv1_path, omm::Point::Direction::Forward}, omm::PolarCoordinates());

  ASSERT_NO_THROW(make_copy());

  ASSERT_NO_FATAL_FAILURE(assert_valid());
}

TEST_F(PathVectorCopy, OP)
{
  auto pv1_p0 = std::make_shared<omm::PathPoint>(omm::Point({0, 0}), &pv_original);
  auto& pv1_path = pv_original.add_path();
  pv1_path.set_single_point(pv1_p0);
  pv1_p0->geometry().set_tangent({&pv1_path, omm::Point::Direction::Backward}, omm::PolarCoordinates());

  ASSERT_NO_THROW(make_copy());

  ASSERT_NO_FATAL_FAILURE(assert_valid());
}
