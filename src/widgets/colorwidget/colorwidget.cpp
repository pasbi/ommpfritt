#include "widgets/colorwidget/colorwidget.h"
#include "color/color.h"
#include "color/namedcolors.h"
#include "main/application.h"
#include "namedcolorsdialog.h"
#include "scene/scene.h"
#include "ui_colorwidget.h"
#include "widgets/colorwidget/colorcircle.h"

namespace omm
{
ColorWidget::ColorWidget(QWidget* parent) : ColorPicker(parent), m_ui(new Ui::ColorWidget)
{
  m_ui->setupUi(this);

  m_component_widgets = {
      {Color::Role::Red, {m_ui->sl_r, m_ui->sb_r}},
      {Color::Role::Green, {m_ui->sl_g, m_ui->sb_g}},
      {Color::Role::Blue, {m_ui->sl_b, m_ui->sb_b}},
      {Color::Role::Hue, {m_ui->sl_h, m_ui->sb_h}},
      {Color::Role::Saturation, {m_ui->sl_s, m_ui->sb_s}},
      {Color::Role::Value, {m_ui->sl_v, m_ui->sb_v}},
      {Color::Role::Alpha, {m_ui->sl_a, m_ui->sb_a}},
  };

  for (auto&& [role, cwidgets] : m_component_widgets) {
    for (AbstractColorComponentWidget* cwidget : cwidgets) {
      cwidget->set_role(role);
      connect(cwidget, &AbstractColorComponentWidget::color_changed, this, &ColorWidget::set_color);
      connect(this, &ColorWidget::color_changed, cwidget, &AbstractColorComponentWidget::set_color);
    }
  }

  add_color_picker(std::make_unique<ColorCircle>());
  connect(m_ui->pb_named_colors,
          &QPushButton::clicked,
          this,
          &ColorWidget::show_named_colors_dialog);

  NamedColors& model = Application::instance().scene->named_colors();
  using ComboBoxNCHPM = NamedColorsHighlighProxyModel<QComboBox>;
  auto cb_proxy = std::make_unique<ComboBoxNCHPM>(model, *m_ui->cb_named_colors->view());
  m_ui->cb_named_colors->set_model(*cb_proxy);
  cb_proxy.release()->setParent(m_ui->cb_named_colors);
  connect(m_ui->cb_named_colors, &ComboBox::current_index_changed, [this](int index) {
    if (index >= 0) {
      const QString name = m_ui->cb_named_colors->view()->itemText(index);
      set_color(Color(name));
    }
  });
}

ColorWidget::~ColorWidget() = default;

QString ColorWidget::name() const
{
  return tr("ColorWidget");
}

void ColorWidget::set_compact()
{
  m_ui->w_bonus->hide();
}

void ColorWidget::hide_named_colors()
{
  m_ui->cb_named_colors->hide();
  m_ui->pb_named_colors->hide();
}

void ColorWidget::set_color(const Color& color)
{
  ColorPicker::set_color(color);
  for (auto&& picker : m_color_pickers) {
    QSignalBlocker blocker(picker);
    picker->set_color(color);
  }

  for (auto&& [role, cwidgets] : m_component_widgets) {
    for (AbstractColorComponentWidget* cwidget : cwidgets) {
      QSignalBlocker blocker(cwidget);
      cwidget->set_color(color);
    }
  }

  {
    QSignalBlocker bloker(m_ui->le_html);
    m_ui->le_html->setText(color.to_html());
  }

  m_ui->w_color->set_new_color(color);
  if (color.model() == Color::Model::Named) {
    m_ui->cb_named_colors->set_current_text(color.name());
  } else {
    m_ui->cb_named_colors->set_current_index(-1);
  }
}

void ColorWidget::add_color_picker(std::unique_ptr<ColorPicker> picker)
{
  m_color_pickers.push_back(picker.get());
  connect(picker.get(), &ColorPicker::color_changed, this, &ColorWidget::set_color);
  connect(this, &ColorWidget::color_changed, picker.get(), &ColorPicker::set_color);
  m_ui->cb_color_widget->addItem(picker->name());
  m_ui->sw_color_widgets->addWidget(picker.release());
}

void ColorWidget::show_named_colors_dialog()
{
  const Color color = this->color();
  Color current_ordinary = color;
  current_ordinary.to_ordinary_color();
  NamedColorsDialog().exec();
  update();
  if (color.model() == Color::Model::Named) {
    NamedColors& model = Application::instance().scene->named_colors();
    if (!model.has_color(color.name())) {
      set_color(current_ordinary);
    }
  }
}

}  // namespace omm
