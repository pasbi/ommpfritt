#include "enumnames.h"
#include "common.h"

namespace omm
{
template<> const char* enum_name_impl<Kind>(Kind e)
{
  switch (e) {
  case Kind::Tag:
    return QT_TRANSLATE_NOOP("Enum", "Tag");
  case Kind::Node:
    return QT_TRANSLATE_NOOP("Enum", "Node");
  case Kind::Object:
    return QT_TRANSLATE_NOOP("Enum", "Object");
  case Kind::Style:
    return QT_TRANSLATE_NOOP("Enum", "Style");
  case Kind::Tool:
    return QT_TRANSLATE_NOOP("Enum", "Tool");
  default:
    Q_UNREACHABLE();
    return "";
  }
}

template<> std::set<Kind> enumerate_enum<Kind>()
{
  return {Kind::Tag, Kind::Node, Kind::Object, Kind::Style, Kind::Tool};
}

template<> bool is_flag<Kind>()
{
  return true;
}

template<> const char* enum_name_impl<Flag>(Flag f)
{
  switch (f) {
  case Flag::Convertible:
    return QT_TRANSLATE_NOOP("Enum", "Convertible");
  case Flag::HasScript:
    return QT_TRANSLATE_NOOP("Enum", "HasScript");
  case Flag::IsView:
    return QT_TRANSLATE_NOOP("Enum", "IsView");
  case Flag::HasPythonNodes:
    return QT_TRANSLATE_NOOP("Enum", "HasPythonNodes");
  case Flag::HasGLSLNodes:
    return QT_TRANSLATE_NOOP("Enum", "HasGLSLNodes");
  default:
    Q_UNREACHABLE();
    return "";
  }
}

template<> std::set<Flag> enumerate_enum<Flag>()
{
  return {Flag::Convertible,
          Flag::HasScript,
          Flag::IsView,
          Flag::HasPythonNodes,
          Flag::HasGLSLNodes};
}

template<> bool is_flag<Flag>()
{
  return true;
}

}  // namespace omm
