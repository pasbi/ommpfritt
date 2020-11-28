#include "preferences/viewportpage.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <memory>

namespace
{
const std::vector<std::pair<Qt::MouseButton, QString>> button_map{
    {Qt::LeftButton, QT_TRANSLATE_NOOP("omm::ViewportPage", "Left")},
    {Qt::MiddleButton, QT_TRANSLATE_NOOP("omm::ViewportPage", "Middle")},
    {Qt::RightButton, QT_TRANSLATE_NOOP("omm::ViewportPage", "Right")},
};

const std::vector<std::pair<Qt::KeyboardModifier, QString>> modifier_map{
    {Qt::ShiftModifier, QT_TRANSLATE_NOOP("omm::ViewportPage", "Shift")},
    {Qt::ControlModifier, QT_TRANSLATE_NOOP("omm::ViewportPage", "Ctrl")},
    {Qt::MetaModifier, QT_TRANSLATE_NOOP("omm::ViewportPage", "Meta")},
    {Qt::AltModifier, QT_TRANSLATE_NOOP("omm::ViewportPage", "Alt")},
};

void set_size_policy(QWidget& widget)
{
  widget.setSizePolicy(QSizePolicy::Ignored, widget.sizePolicy().verticalPolicy());
}

}  // namespace

namespace omm
{
ViewportPage::MouseModifiersGroup::MouseModifiersGroup(Preferences::MouseModifier& model,
                                                       QFormLayout& layout)
    : m_model(model)
{
  auto cb = std::make_unique<QComboBox>();
  for (const auto& [_, label] : button_map) {
    cb->addItem(tr(label.toUtf8().constData()));
  }
  const auto it = std::find_if(button_map.begin(), button_map.end(), [&model](const auto& pair) {
    return pair.first == model.button;
  });
  cb->setCurrentIndex(std::distance(button_map.begin(), it));
  m_button_cb = cb.get();

  auto label = std::make_unique<QLabel>(model.label);
  label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  label->setBuddy(m_button_cb);
  auto hlayout = std::make_unique<QHBoxLayout>();
  hlayout->setContentsMargins(0, 0, 0, 0);
  set_size_policy(*cb);
  hlayout->addWidget(cb.release());
  for (const auto& i : modifier_map) {
    auto cb = std::make_unique<QCheckBox>();
    m_modifier_cbs.insert({i.first, cb.get()});
    if (!!(model.modifiers & i.first)) {
      cb->setChecked(true);
    }
    auto cb_layout = std::make_unique<QHBoxLayout>();
    cb_layout->setAlignment(Qt::AlignHCenter);
    cb_layout->setContentsMargins(0, 0, 0, 0);
    cb_layout->addWidget(cb.release());
    hlayout->addLayout(cb_layout.release());
  }

  layout.addRow(label.release(), hlayout.release());
}

ViewportPage::GridGroup::GridGroup(Preferences::GridOption& model, QFormLayout& layout)
    : m_model(model)
{
  static constexpr double MAX_PEN_WIDTH = 10;
  auto sp_penwidth = std::make_unique<QDoubleSpinBox>();
  sp_penwidth->setRange(0.0, MAX_PEN_WIDTH);
  sp_penwidth->setSuffix(tr(" px"));
  sp_penwidth->setValue(model.pen_width);
  m_sp_penwidth = sp_penwidth.get();

  auto cb_style = std::make_unique<QComboBox>();
  cb_style->addItem(tr("None"));
  cb_style->addItem(tr("Solid"));
  cb_style->addItem(tr("Dashed"));
  cb_style->addItem(tr("Dotted"));
  cb_style->addItem(tr("DashDotted"));
  cb_style->addItem(tr("DashDotDotted"));
  cb_style->setCurrentIndex(model.pen_style);
  m_cb_penstyle = cb_style.get();

  auto cb_zorder = std::make_unique<QComboBox>();
  cb_zorder->addItem(tr("Invisible"));
  cb_zorder->addItem(tr("Foreground"));
  cb_zorder->addItem(tr("Background"));
  cb_zorder->setCurrentIndex(static_cast<int>(model.zorder));
  m_cb_zorder = cb_zorder.get();

  auto label = std::make_unique<QLabel>(model.label);
  label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  label->setBuddy(sp_penwidth.get());

  auto hlayout = std::make_unique<QHBoxLayout>();
  hlayout->addWidget(sp_penwidth.release());
  hlayout->addWidget(cb_style.release());
  hlayout->addWidget(cb_zorder.release());
  layout.addRow(label.release(), hlayout.release());
}

void ViewportPage::GridGroup::apply()
{
  m_model.pen_style = static_cast<Qt::PenStyle>(m_cb_penstyle->currentIndex());
  m_model.pen_width = m_sp_penwidth->value();
  m_model.zorder = static_cast<decltype(m_model.zorder)>(m_cb_zorder->currentIndex());
}

void ViewportPage::MouseModifiersGroup::apply()
{
  m_model.button = button_map.at(m_button_cb->currentIndex()).first;
  m_model.modifiers = {};
  for (std::size_t i = 0; i < m_modifier_cbs.size(); ++i) {
    const auto modifier = modifier_map.at(i).first;
    if (m_modifier_cbs.at(modifier)->isChecked()) {
      m_model.modifiers |= modifier;
    }
  }
}

ViewportPage::ViewportPage(Preferences& preferences) : m_preferences(preferences)
{
  auto layout = std::make_unique<QFormLayout>();

  auto mm_header_layout = std::make_unique<QHBoxLayout>();
  auto label = std::make_unique<QLabel>(tr("Button"));
  label->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
  set_size_policy(*label);
  mm_header_layout->addWidget(label.release());
  for (const auto& i : modifier_map) {
    auto label = std::make_unique<QLabel>(tr(i.second.toUtf8().constData()));
    label->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
    set_size_policy(*label);
    mm_header_layout->addWidget(label.release());
  }
  layout->addRow("", mm_header_layout.release());
  for (auto& [key, mm] : m_preferences.mouse_modifiers) {
    m_mouse_modifiers.insert({key, MouseModifiersGroup(mm, *layout)});
  }
  for (auto& [key, go] : m_preferences.grid_options) {
    m_grid_options.insert({key, GridGroup(go, *layout)});
  }

  setLayout(layout.release());
}

ViewportPage::~ViewportPage() = default;

void ViewportPage::about_to_accept()
{
  for (auto&& [key, value] : m_mouse_modifiers) {
    value.apply();
  }
  for (auto&& [key, value] : m_grid_options) {
    value.apply();
  }
}

}  // namespace omm
