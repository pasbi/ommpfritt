#include "widgets/colorwidget/colorwidget.h"
#include "mainwindow/application.h"
#include "color/namedcolors.h"
#include "widgets/colorwidget/colorcircle.h"
#include "ui_colorwidget.h"
#include "color/color.h"
#include "namedcolorsdialog.h"

namespace omm
{

ColorWidget::ColorWidget(QWidget* parent)
  : ColorPicker(parent), m_ui(new Ui::ColorWidget)
{
  m_ui->setupUi(this);

  m_component_widgets = {
    { Color::Role::Red,        { m_ui->sl_r, m_ui->sb_r } },
    { Color::Role::Green,      { m_ui->sl_g, m_ui->sb_g } },
    { Color::Role::Blue,       { m_ui->sl_b, m_ui->sb_b } },
    { Color::Role::Hue,        { m_ui->sl_h, m_ui->sb_h } },
    { Color::Role::Saturation, { m_ui->sl_s, m_ui->sb_s } },
    { Color::Role::Value,      { m_ui->sl_v, m_ui->sb_v } },
    { Color::Role::Alpha,      { m_ui->sl_a, m_ui->sb_a } },
  };

  for (auto&& [role, cwidgets] : m_component_widgets) {
    for (AbstractColorComponentWidget* cwidget : cwidgets) {
      cwidget->set_role(role);
      connect(cwidget, SIGNAL(color_changed(const Color&)), this, SLOT(set_color(const Color&)));
      connect(this, SIGNAL(color_changed(const Color&)), cwidget, SLOT(set_color(const Color&)));
    }
  }

  add_color_picker(std::make_unique<ColorCircle>());
  connect(m_ui->pb_named_colors, SIGNAL(clicked()), this, SLOT(show_named_colors_dialog()));

  NamedColors& model = Application::instance().scene.named_colors();
  using ComboBoxNCHPM = NamedColorsHighlighProxyModel<QComboBox>;
  auto cb_proxy = std::make_unique<ComboBoxNCHPM>(model, *m_ui->cb_named_colors);
  m_ui->cb_named_colors->setModel(cb_proxy.get());
  cb_proxy.release()->setParent(m_ui->cb_named_colors);
  connect(m_ui->cb_named_colors, qOverload<const QString&>(&QComboBox::currentIndexChanged),
          [this](const QString& name) {
    set_color(Color(name.toStdString()));
  });
}

ColorWidget::~ColorWidget()
{
}

std::string ColorWidget::name() const
{
  return tr("ColorWidget").toStdString();
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
    m_ui->le_html->setText(QString::fromStdString(color.to_html()));
  }

  m_ui->w_color->set_new_color(color);
  if (color.model() == Color::Model::Named) {
    m_ui->cb_named_colors->setCurrentText(QString::fromStdString(color.name()));
  } else {
    m_ui->cb_named_colors->setCurrentIndex(-1);
  }
}

void ColorWidget::add_color_picker(std::unique_ptr<ColorPicker> picker)
{
  m_color_pickers.push_back(picker.get());
  connect(picker.get(), SIGNAL(color_changed(const Color&)), this, SLOT(set_color(const Color&)));
  connect(this, SIGNAL(color_changed(const Color&)), picker.get(), SLOT(set_color(const Color&)));
  m_ui->cb_color_widget->addItem(QString::fromStdString(picker->name()));
  m_ui->sw_color_widgets->addWidget(picker.release());
}

void ColorWidget::show_named_colors_dialog()
{
  NamedColorsDialog().exec();
  update();
}


}  // namespace omm
