#include "preferences/viewportpage.h"
#include <QGridLayout>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <memory>
#include <QFormLayout>

namespace
{

const std::vector<std::pair<Qt::MouseButton, QString>> button_map {
  { Qt::LeftButton, QT_TRANSLATE_NOOP("omm::ViewportPage","Left") },
  { Qt::MiddleButton, QT_TRANSLATE_NOOP("omm::ViewportPage","Middle") },
  { Qt::RightButton, QT_TRANSLATE_NOOP("omm::ViewportPage","Right") },
};

const std::vector<std::pair<Qt::KeyboardModifier, QString>> modifier_map {
  { Qt::ShiftModifier, QT_TRANSLATE_NOOP("omm::ViewportPage","Shift") },
  { Qt::ControlModifier, QT_TRANSLATE_NOOP("omm::ViewportPage","Ctrl") },
  { Qt::MetaModifier, QT_TRANSLATE_NOOP("omm::ViewportPage","Meta") },
  { Qt::AltModifier, QT_TRANSLATE_NOOP("omm::ViewportPage","Alt") },
};

void set_size_policy(QWidget& widget)
{
  widget.setSizePolicy(QSizePolicy::Ignored, widget.sizePolicy().verticalPolicy());
}

std::unique_ptr<QWidget> center(std::unique_ptr<QWidget> widget)
{
}

}  // namespace

namespace omm
{

ViewportPage::MouseModifiersGroup::
MouseModifiersGroup(Preferences::MouseModifier& model, QFormLayout& layout)
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
  for (std::size_t i = 0; i < modifier_map.size(); ++i) {
    auto cb = std::make_unique<QCheckBox>();
    m_modifier_cbs.insert({ modifier_map.at(i).first, cb.get() });
    if (!!(model.modifiers & modifier_map.at(i).first)) {
      cb->setChecked(true);
    }
    auto layout = std::make_unique<QHBoxLayout>();
    layout->setAlignment(Qt::AlignHCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(cb.release());
    hlayout->addLayout(layout.release());
  }

  layout.addRow(label.release(), hlayout.release());
}

ViewportPage::GridGroup::GridGroup(Preferences::GridOption& model, QFormLayout& layout)
  : m_model(model)
{

}

void ViewportPage::MouseModifiersGroup::apply()
{
  m_model.button = button_map.at(m_button_cb->currentIndex()).first;
  m_model.modifiers = 0;
  for (std::size_t i = 0; i < m_modifier_cbs.size(); ++i) {
    const auto modifier = modifier_map.at(i).first;
    if (m_modifier_cbs.at(modifier)->isChecked()) {
      m_model.modifiers |= modifier;
    }
  }
}

ViewportPage::ViewportPage(Preferences& preferences)
  : m_preferences(preferences)
{
  auto layout = std::make_unique<QFormLayout>();

  auto mm_header_layout = std::make_unique<QHBoxLayout>();
  auto label = std::make_unique<QLabel>(tr("Button"));
  label->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
  set_size_policy(*label);
  mm_header_layout->addWidget(label.release());
  for (std::size_t i = 0; i < modifier_map.size(); ++i) {
    auto label = std::make_unique<QLabel>(tr(modifier_map.at(i).second.toUtf8().constData()));
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

ViewportPage::~ViewportPage()
{
}

void ViewportPage::about_to_accept()
{
  for (auto&& [ key, value ] : m_mouse_modifiers) {
    value.apply();
  }
}



}  // namespace
