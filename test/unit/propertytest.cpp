#include "properties/propertyfilter.h"
#include <gtest/gtest.h>

TEST(Property, ReferenceFilter)
{
  using namespace omm;
  const PropertyFilter any_object({ Kind::Object }, {{}});

  EXPECT_TRUE(any_object.accepts(Kind::Object, Flag::None));
  EXPECT_TRUE(any_object.accepts(Kind::Object, Flag::IsView));
  EXPECT_TRUE(any_object.accepts(Kind::Object, Flag::IsView | Flag::HasNodes));
  EXPECT_TRUE(any_object.accepts(Kind::Object, Flag::HasPython));
  EXPECT_TRUE(any_object.accepts(Kind::Object, Flag::HasPython | Flag::Convertible));
  EXPECT_TRUE(any_object.accepts(Kind::Object, Flag::Convertible));

  EXPECT_FALSE(any_object.accepts(Kind::Node, Flag::None));
  EXPECT_FALSE(any_object.accepts(Kind::Tag, Flag::IsView));
  EXPECT_FALSE(any_object.accepts(Kind::Tool, Flag::IsView | Flag::HasNodes));
  EXPECT_FALSE(any_object.accepts(Kind::Node, Flag::HasPython));
  EXPECT_FALSE(any_object.accepts(Kind::Tag, Flag::HasPython | Flag::Convertible));
  EXPECT_FALSE(any_object.accepts(Kind::Style, Flag::Convertible));
}
