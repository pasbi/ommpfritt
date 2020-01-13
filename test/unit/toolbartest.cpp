#include "mainwindow/toolbar.h"
#include "common.h"
#include "logging.h"
#include <gtest/gtest.h>

TEST(toolbar, split)
{
  EXPECT_EQ(omm::ToolBar::split("A;B;C"), QStringList({ "A", "B", "C" }));
  EXPECT_EQ(omm::ToolBar::split("foo;bar;foobar"), QStringList({ "foo", "bar", "foobar" }));
  EXPECT_EQ(omm::ToolBar::split("foobar;foobar"), QStringList({ "foobar", "foobar" }));
  EXPECT_EQ(omm::ToolBar::split("foo,bar;foobar"), QStringList({ "foo,bar", "foobar" }));
  EXPECT_EQ(omm::ToolBar::split("[foo;bar];foobar"), QStringList({ "[foo;bar]", "foobar" }));
  EXPECT_EQ(omm::ToolBar::split(";;"), QStringList({ "", "", "" }));
  EXPECT_EQ(omm::ToolBar::split(""), QStringList({ }));
  EXPECT_EQ(omm::ToolBar::split("erfrf;;pop13"), QStringList({ "erfrf", "", "pop13" }));
  EXPECT_EQ(omm::ToolBar::split("undo;redo;[Ellipse;RectangleObject];Instance"),
            QStringList({ "undo", "redo", "[Ellipse;RectangleObject]", "Instance" }));

}
