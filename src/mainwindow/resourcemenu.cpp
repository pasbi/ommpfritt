#include "mainwindow/resourcemenu.h"

#include <QFile>
#include <QSettings>
#include <QActionGroup>
#include <memory>
#include <QDirIterator>
#include <QLocale>
#include <QMessageBox>
#include <QApplication>

namespace omm
{

template<typename PluginT> ResourceMenu<PluginT>::ResourceMenu()
  : QMenu(PluginT::menu_label())
{
  QSettings settings;

  auto action_group = std::make_unique<QActionGroup>(this).release();
  action_group->setExclusive(true);
  bool anything_selected = false;
  const auto current_value = settings.value(PluginT::SETTINGS_KEY);
  for (const std::string& key : available_keys()) {
    if (!key.empty()) {
      const QString label = PluginT::label(key);
      const typename PluginT::value_type value = PluginT::value(key);
      auto* action = addAction(PluginT::label(key));
      action_group->addAction(action);
      action->setCheckable(true);
      const bool current_value_selected = value == current_value;
      action->setChecked(current_value_selected);
      connect(action, &QAction::triggered, [this, value]() {
        QSettings().setValue(PluginT::SETTINGS_KEY, value);
        PluginT::on_value_changed(this, value);
      });
      assert(!current_value_selected || !anything_selected);
      anything_selected |= current_value_selected;
    }
  }
  addSeparator();
  auto* default_action = addAction(MainWindow::tr("default"));
  default_action->setCheckable(true);
  default_action->setChecked(!anything_selected);
  connect(default_action, &QAction::triggered, [this]() {
    QSettings().remove(PluginT::SETTINGS_KEY);
    PluginT::on_value_changed(this, typename PluginT::value_type());
  });
  action_group->addAction(default_action);
}

template<typename PluginT> std::vector<std::string> ResourceMenu<PluginT>::available_keys()
{
  using namespace std::string_literals;
  std::list<std::string> trs;
  QDirIterator it(PluginT::RESOURCE_DIRECTORY, QDirIterator::Subdirectories);
  static const QString prefix = QString::fromStdString(  PluginT::RESOURCE_DIRECTORY + "/"s
                                                       + PluginT::RESOURCE_PREFIX + "_"s);
  static const QString suffix(PluginT::RESOURCE_SUFFIX);
  while (it.hasNext()) {
    const auto filename = it.next();
    if (filename.startsWith(prefix) && filename.endsWith(suffix)) {
      const int code_length = filename.size() - prefix.size() - suffix.size();
      if (code_length < 0) {
        trs.push_back("");
      } else {
        const auto code = filename.mid(prefix.size(), code_length);
        trs.push_back(code.toStdString());
      }
    }
  }

  return std::vector(trs.begin(), trs.end());
}


void LanguagePlugin::on_value_changed(QMenu *menu, const QLocale &value)
{
  Q_UNUSED(value);
  const auto msg = MainWindow::tr(
      "Changing language takes effect after restarting the application.");
  QMessageBox::information(menu, MainWindow::tr("information"), msg);
}

QString LanguagePlugin::label(const std::string &key)
{
  const auto language = QLocale(key.c_str()).language();
  return QMainWindow::tr(QLocale::languageToString(language).toStdString().c_str());
}

QLocale LanguagePlugin::value(const std::string& key)
{
  return QLocale(QString::fromStdString(key));
}

QString LanguagePlugin::menu_label()
{
  return MainWindow::tr("Language");
}

void SkinPlugin::on_value_changed(QMenu *menu, const QString &value)
{
  Q_UNUSED(menu);
  load_skin(value);
}

QString SkinPlugin::label(const std::string &key)
{
  return QString::fromStdString(key);
}

QString SkinPlugin::value(const std::string &key)
{
  return QString("%1/%2_%3%4")
      .arg(RESOURCE_DIRECTORY)
      .arg(RESOURCE_PREFIX)
      .arg(key.c_str())
      .arg(RESOURCE_SUFFIX);
}

QString SkinPlugin::menu_label()
{
  return MainWindow::tr("Skin");
}

void SkinPlugin::load_skin(const QString &filename)
{
  if (filename.isEmpty()) {
    qApp->setStyleSheet("");
  } else {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      LWARNING << "Failed to read skin '" << filename << "'.";
      return;
    }
    QTextStream stream(&file);
    qApp->setStyleSheet(stream.readAll());
  }
}

template class ResourceMenu<LanguagePlugin>;
template class ResourceMenu<SkinPlugin>;

}  // namespace omm
