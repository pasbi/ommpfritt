#include "gtest/gtest.h"
#include <cassert>

#include "stringinterpolation.h"
#include "logging.h"

std::string interpolate(const QString& pattern, const omm::StringInterpolation::map_type& values)
{
  return static_cast<QString>(omm::StringInterpolation{pattern, values}).toStdString();
}

TEST(StringInterpolation, InvalidFormat)
{
  using InvalidFormatException = omm::StringInterpolation::InvalidFormatException;
  EXPECT_THROW(interpolate("{", {}), InvalidFormatException);
  EXPECT_THROW(interpolate("}", {}), InvalidFormatException);
  EXPECT_THROW(interpolate("{}", {}), InvalidFormatException);
  EXPECT_THROW(interpolate("{foo}", {}), InvalidFormatException);
  EXPECT_THROW(interpolate("a{foo}b", {}), InvalidFormatException);
  EXPECT_THROW(interpolate("{{foo}b", {{"foo", 1}}), InvalidFormatException);
}

TEST(StringInterpolation, ReplaceIntSimple)
{
  EXPECT_EQ("foo42baz", interpolate("foo{bar}baz", {{"bar", 42}}));
  EXPECT_EQ("-1foo", interpolate("{xx}foo", {{"xx", -1}}));
  EXPECT_EQ("-1foo12", interpolate("{xx}foo{y}", {{"xx", -1}, {"y", 12}}));
  EXPECT_EQ("-1-112", interpolate("{xx}{xx}{y}", {{"xx", -1}, {"y", 12}}));
  EXPECT_EQ("-1-1-1-1-1@-1", interpolate("{xx}{xx}{xx}{xx}{xx}@{xx}", {{"xx", -1}, {"y", 12}}));
  EXPECT_EQ("abc", interpolate("abc", {{"xx", -1}, {"y", 12}}));
  EXPECT_EQ("", interpolate("", {{"xx", -1}, {"y", 12}}));
}

TEST(StringInterpolation, ReplaceIntFormat)
{
  EXPECT_EQ("foo0042baz", interpolate("foo{bar:04}baz", {{"bar", 42}}));
  EXPECT_EQ("foo42baz", interpolate("foo{bar:01}baz", {{"bar", 42}}));
  EXPECT_EQ("xxxx42", interpolate("{bar:x6}", {{"bar", 42}}));
  EXPECT_EQ("-042", interpolate("{bar:04}", {{"bar", -42}}));
  EXPECT_EQ("-42", interpolate("{bar:@3}", {{"bar", -42}}));
}

TEST(StringInterpolation, ReplaceString)
{
  EXPECT_EQ("foo-bar-baz", interpolate("foo{bar}baz", {{"bar", "-bar-"}}));
  EXPECT_EQ("-bar-baz", interpolate("{bar}baz", {{"bar", "-bar-"}}));
  EXPECT_EQ("-bar-", interpolate("{bar}", {{"bar", "-bar-"}}));
}
