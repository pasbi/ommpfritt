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

template<> std::set<Kind> enumerate_enum<Kind>() {
  return {
    Kind::Tag, Kind::Node, Kind::Object, Kind::Style, Kind::Tool
  };
}

template<> bool is_flag<Kind>() { return true; }

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

template<> std::set<Flag> enumerate_enum<Flag>() {
  return {
    Flag::Convertible, Flag::HasScript, Flag::IsView, Flag::HasPythonNodes, Flag::HasGLSLNodes
  };
}

template<> bool is_flag<Flag>() { return true; }

template<> const char* enum_name_impl<ExitStatus>(ExitStatus e)
{
  switch (e) {
  case ExitStatus::invalid_input_format:
    return QT_TRANSLATE_NOOP("Enum", "invalid input format");
  case ExitStatus::object_name_not_found:
    return QT_TRANSLATE_NOOP("Enum", "object name not found");
  case ExitStatus::object_type_not_found:
    return QT_TRANSLATE_NOOP("Enum", "object type not found");
  case ExitStatus::refuse_overwrite_file:
    return QT_TRANSLATE_NOOP("Enum", "refuse overwrite file");
  case ExitStatus::non_unique_object_reference:
    return QT_TRANSLATE_NOOP("Enum", "non unique object reference");
  default:
    Q_UNREACHABLE();
    return "";
  }
}

template<> std::set<ExitStatus> enumerate_enum<ExitStatus>() {
  return {
    ExitStatus::invalid_input_format, ExitStatus::object_name_not_found,
    ExitStatus::object_type_not_found, ExitStatus::refuse_overwrite_file,
    ExitStatus::non_unique_object_reference
  };
}

template<> bool is_flag<ExitStatus>() { return false; }

}  // namespace omm
