#include "preferences/viewportpage.h"
#include <QGridLayout>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <memory>

namespace
{

const std::vector<std::pair<Qt::MouseButton, QString>> button_map {
  { Qt::LeftButton, QT_TRANSLATE_NOOP("Preferences","Left") },
  { Qt::MiddleButton, QT_TRANSLATE_NOOP("Preferences","Middle") },
  { Qt::RightButton, QT_TRANSLATE_NOOP("Preferences","Right") },
};

const std::vector<std::pair<Qt::KeyboardModifier, QString>> modifier_map {
  { Qt::ShiftModifier, QT_TRANSLATE_NOOP("Preferences","Shift") },
  { Qt::ControlModifier, QT_TRANSLATE_NOOP("Preferences","Ctrl") },
  { Qt::MetaModifier, QT_TRANSLATE_NOOP("Preferences","Meta") },
  { Qt::AltModifier, QT_TRANSLATE_NOOP("Preferences","Alt") },
};

}  // namespace

namespace omm
{

ViewportPage::MouseModifiersGroup::
MouseModifiersGroup(Preferences::MouseModifier& model, QGridLayout& layout, int& row)
  : m_model(model)
{
  if (row == 0) {
    layout.addWidget(new QLabel("Button"), row, 1, 1, 1, Qt::AlignLeft);
    for (std::size_t i = 0; i < modifier_map.size(); ++i) {
      layout.addWidget(new QLabel(modifier_map.at(i).second), row, 2 + i, 1, 1, Qt::AlignCenter);
    }
    row += 1;
  }

  auto cb = std::make_unique<QComboBox>();
  for (const auto& [_, label] : button_map) {
    cb->addItem(label);
  }
  {
    const auto it = std::find_if(button_map.begin(), button_map.end(), [&model](const auto& pair) {
      return pair.first == model.button;
    });
    cb->setCurrentIndex(std::distance(button_map.begin(), it));
  }
  m_button_cb = cb.get();
  auto label = std::make_unique<QLabel>(model.label);
  label->setBuddy(m_button_cb);
  layout.addWidget(label.release(), row, 0, 1, 1, Qt::AlignLeft);
  layout.addWidget(cb.release(), row, 1, 1, 1, Qt::AlignLeft);
  for (std::size_t i = 0; i < modifier_map.size(); ++i) {
    auto cb = std::make_unique<QCheckBox>();
    m_modifier_cbs.insert({ modifier_map.at(i).first, cb.get() });
    if (!!(model.modifiers & modifier_map.at(i).first)) {
      cb->setChecked(true);
    }
    layout.addWidget(cb.release(), row, 2+i, 1, 1, Qt::AlignCenter);
  }
  row += 1;
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
  auto layout = std::make_unique<QVBoxLayout>();
  auto grid_layout = std::make_unique<QGridLayout>();
  {
    int row = 0;
    for (auto& [key, mm] : m_preferences.mouse_modifiers()) {
      m_mouse_modifiers.insert({key, MouseModifiersGroup(mm, *grid_layout, row)});
    }
  }
  layout->addLayout(grid_layout.release());
  layout->addStretch();
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
