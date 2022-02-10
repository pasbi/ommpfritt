#include "preferences/uicolors.h"
#include "color/color.h"
#include "logging.h"
#include "preferences/preferencestreeitem.h"
#include "main/application.h"
#include <QApplication>
#include <QColor>
#include <QCoreApplication>
#include <QPainter>
#include <QPalette>
#include <QProxyStyle>
#include <QSettings>
#include <QStyleFactory>
#include <QWidget>

namespace
{

const std::map<QPalette::ColorRole, QString> role_map{
    {QPalette::Window, "window"},
    {QPalette::WindowText, "window text"},
    {QPalette::Text, "text"},
    {QPalette::Base, "base"},
    {QPalette::AlternateBase, "alternate base"},
    {QPalette::PlaceholderText, "placeholder text"},
    {QPalette::Button, "button"},
    {QPalette::ButtonText, "button text"},
    {QPalette::Light, "light"},
    {QPalette::Midlight, "midlight"},
    {QPalette::Dark, "dark"},
    {QPalette::Mid, "mid"},
    {QPalette::Shadow, "shadow"},
    {QPalette::BrightText, "bright text"},
    {QPalette::Highlight, "highlight"},
    {QPalette::HighlightedText, "highlighted text"},
    {QPalette::Link, "link"},
    {QPalette::LinkVisited, "link visited"},
};

const std::map<std::size_t, QPalette::ColorGroup> group_map{{0, QPalette::Active},
                                                            {1, QPalette::Inactive},
                                                            {2, QPalette::Disabled}};

omm::Color color_from_html(const QString& html)
{
  bool ok = false;
  auto color = omm::Color::from_html(html, &ok);
  assert(ok);
  return color;
}

}  // namespace

namespace omm
{
UiColors::UiColors() : PreferencesTree("uicolors", ":/uicolors/ui-colors-dark.cfg")
{
  // set this macro to print Qt's default color.
  //#define PRINT_DEFAULT_COLOR_SCHEMA

#ifdef PRINT_DEFAULT_COLOR_SCHEMA
  for (auto&& [role, name] : role_map) {
    std::cout << name << ": "
              << Color(QApplication::palette().color(QPalette::Active, role)).to_hex() << "/"
              << Color(qApp->palette().color(QPalette::Active, role)).to_hex() << "/"
              << Color(qApp->palette().color(QPalette::Active, role)).to_hex() << std::endl;
  }
#endif  // PRINT_DEFAULT_COLOR_SCHEMA
  m_skins.push_back({tr("dark"), ":/uicolors/ui-colors-dark.cfg", "ui-colors-dark"});
  m_skins.push_back({tr("light"), ":/uicolors/ui-colors-light.cfg", "ui-colors-light"});
  set_skin(QSettings().value("ui-colors-schema", 0).toInt());
}

UiColors::~UiColors()
{
  save_in_qsettings(m_skins.at(m_current_skin_index).qsettings_group);
  QSettings().setValue("ui-colors-schema", m_current_skin_index);
}

QVariant UiColors::data(int column, const PreferencesTreeValueItem& item, int role) const
{
  switch (role) {
  case Qt::DisplayRole:
    return QVariant();
  case Qt::BackgroundRole:
    return color_from_html(item.value(column - 1)).to_qcolor();
  case Qt::EditRole:
    return color_from_html(item.value(column - 1)).to_html();
  case DEFAULT_VALUE_ROLE:
    return color_from_html(item.default_value(column - 1)).to_html();
  default:
    return QVariant();
  }
}

bool UiColors::set_data(int column, PreferencesTreeValueItem& item, const QVariant& value)
{
  item.set_value(Color::from_qcolor(value.value<QColor>()).to_html(), column - 1);
  return true;
}

QPalette UiColors::make_palette() const
{
  const auto get_widget_color = [this](std::size_t column, const QString& name) {
    return color_from_html(value("Widget", name)->value(column)).to_qcolor();
  };
  QPalette palette = QApplication::palette();
  for (auto&& [role, name] : role_map) {
    for (auto&& [column, group] : group_map) {
      palette.setBrush(group, role, get_widget_color(column, name));
    }
  }
  return palette;
}

int UiColors::columnCount(const QModelIndex&) const
{
  return 4;
}

QVariant UiColors::headerData(int section, Qt::Orientation orientation, int role) const
{
  Q_UNUSED(orientation)
  switch (role) {
  case Qt::DisplayRole:
    switch (section) {
    case 0:
      return tr("");
    case 1:
      return tr("Active");
    case 2:
      return tr("Inactive");
    case 3:
      return tr("Disabled");
    default:
      Q_UNREACHABLE();
      return "";
    }
  default:
    return QVariant();
  }
}

Color UiColors ::color(QPalette::ColorGroup pgroup, const QString& group, const QString& name) const
{
  return color_from_html(stored_value(group, name, group_map.at(pgroup)));
}

void UiColors::apply()
{
  PreferencesTree::apply();

  class Style : public QProxyStyle
  {
  public:
    explicit Style(const UiColors& self) : QProxyStyle(QStyleFactory::create("fusion")), self(self) {}
    void polish(QPalette& palette) override { palette = self.make_palette(); }
  private:
    const UiColors& self;
  };
  QApplication::setStyle(std::make_unique<Style>(*this).release());
}

QStringList UiColors::skin_names() const
{
  return util::transform<QList>(m_skins, [](const SkinInfo& info) { return info.display_name; });
}

void UiColors::set_skin(std::size_t index)
{
  if (m_current_skin_index >= 0) {
    save_in_qsettings(m_skins.at(m_current_skin_index).qsettings_group);
  }
  m_current_skin_index = index;
  const auto& current_skin = m_skins.at(m_current_skin_index);
  load_from_file(current_skin.file_name);
  load_from_qsettings(current_skin.qsettings_group);
}

QColor ui_color(const QWidget& widget, const QString& group, const QString& name)
{
  const auto pgroup = widget.isEnabled() ? (widget.window()->isActiveWindow() ? QPalette::Active
                                                                              : QPalette::Inactive)
                                         : QPalette::Disabled;
  return ui_color(pgroup, group, name);
}

QColor ui_color(const QWidget& widget, const QPalette::ColorRole& role)
{
  return ui_color(widget, "Widget", role_map.at(role));
}

void UiColors::draw_background(QPainter& painter, const QRectF& rect)
{
  static const std::array<QColor, 2> bg_colors = {QColor(128, 128, 128), QColor(180, 180, 180)};

  static constexpr int size = 7;

  painter.save();
  int mx = rect.right();
  int my = rect.bottom();
  for (int x = rect.left(); x < rect.right(); x += size) {
    for (int y = rect.top(); y < rect.bottom(); y += size) {
      QRect transformed_rect(QPoint(x, y), QPoint(std::min(mx, x + size), std::min(my, y + size)));
      const auto bg_color
          = bg_colors.at(static_cast<std::size_t>(x / size + y / size) % bg_colors.size());
      painter.fillRect(transformed_rect, bg_color);
    }
  }
  painter.restore();
}

QColor ui_color(const QPalette::ColorGroup& status, const QString& group, const QString& name)
{
  return Application::instance().ui_colors->color(status, group, name).to_qcolor();
}

QColor ui_color(const HandleStatus status, const QString& group, const QString& name)
{
  static const std::map<HandleStatus, QPalette::ColorGroup> color_group_map{
      {HandleStatus::Active, QPalette::Active},
      {HandleStatus::Inactive, QPalette::Inactive},
      {HandleStatus::Hovered, QPalette::Disabled}};
  return ui_color(color_group_map.at(status), group, name);
}

}  // namespace omm
