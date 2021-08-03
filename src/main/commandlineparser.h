#pragma once

#include "logging.h"
#include <QCommandLineParser>
#include <iostream>

namespace omm
{
class CommandLineParser
{
public:
  static constexpr auto TREE_MODE_NAME = "tree";
  static constexpr auto RENDER_MODE_NAME = "render";
  static constexpr auto GUI_MODE_NAME = "gui";
  static constexpr auto MODE_KEY = "mode";
  static constexpr auto VERBOSITY_KEY = "verbosity";
  static constexpr auto NO_OPENGL_KEY = "no-opengl";
  static constexpr auto UNIQUE_KEY = "unique";
  static constexpr auto OUTPUT_KEY = "output";
  static constexpr auto START_FRAME_KEY = "start-frame";
  static constexpr auto SEQUENCE_LENGTH_KEY = "sequence-length";
  static constexpr auto ALLOW_OVERWRITE_KEY = "overwrite";
  static constexpr auto VIEW_NAME_KEY = "view";
  static constexpr auto WIDTH_KEY = "width";
  static constexpr auto OBJECT_PATH_KEY = "object-path";
  static constexpr auto OBJECT_NAME_KEY = "object-name";
  explicit CommandLineParser(const QStringList& args);
  explicit CommandLineParser() = default;

  bool is_set(const QString& name) const;
  template<typename T> T get(const QString& name) const;
  template<typename T> T get(const QString& name, const T& default_value) const
  {
    if (const QString value = parser.value(name); value.isEmpty()) {
      return default_value;
    } else {
      return get<T>(name);
    }
  }

  static constexpr auto FRAMENUMBER_PLACEHOLDER = '#';
  QString scene_filename() const;

private:
  QCommandLineParser parser;
};

}  // namespace omm
