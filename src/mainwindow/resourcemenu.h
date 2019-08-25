#pragma once

#include <set>
#include <QMenu>
#include "mainwindow/mainwindow.h"

namespace omm
{

struct SkinPlugin
{
  using value_type = QString;
  static void on_value_changed(QMenu* menu, const QString& value);
  static QString label(const std::string& key);
  static QString value(const std::string& key);
  static constexpr auto SETTINGS_KEY = MainWindow::SKIN_SETTINGS_KEY;
  static QString menu_label();
  static void load_skin(const QString& filename);
  static constexpr auto RESOURCE_PREFIX = "omm";
  static constexpr auto RESOURCE_SUFFIX = ".qss";
  static constexpr auto RESOURCE_DIRECTORY = ":/skins";
};

struct LanguagePlugin
{
  using value_type = QLocale;
  static void on_value_changed(QMenu* menu, const QLocale &value);
  static QString label(const std::string& key);
  static QLocale value(const std::string& key);
  static constexpr auto SETTINGS_KEY = MainWindow::LOCALE_SETTINGS_KEY;
  static QString menu_label();
  static constexpr auto RESOURCE_PREFIX = "omm";
  static constexpr auto RESOURCE_SUFFIX = ".qm";
  static constexpr auto RESOURCE_DIRECTORY = ":/qm";
};

/**
 * The ResourceMenu class represents a menu which items are determined by the available resources.
 *  It is used, e.g., as a base class for the language and skin menu, where each translation and
 *  each skin are defined by one resource file.
 */
template<typename PluginT>
class ResourceMenu : public QMenu
{
public:
  explicit ResourceMenu();
  static std::vector<std::string> available_keys();
};

using LanguageMenu = ResourceMenu<LanguagePlugin>;
using SkinMenu = ResourceMenu<SkinPlugin>;

}  // namespace
