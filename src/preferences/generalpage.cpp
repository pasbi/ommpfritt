#include "preferences/generalpage.h"

#include <QDirIterator>
#include "logging.h"
#include <QMessageBox>
#include <qsettings.h>
#include <QGridLayout>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>
#include <QFormLayout>
#include "mainwindow/mainwindow.h"

namespace
{

std::vector<QString> languages()
{
  static constexpr auto base_path = ":/qm";
  static constexpr auto prefix = ":/qm/omm_";
  static constexpr auto prefix_length = std::string_view(prefix).length();
  static constexpr auto suffix = ".qm";
  static constexpr auto suffix_length = std::string_view(suffix).length();

  std::list<QString> language_codes;

  QDirIterator it(base_path);
  while (it.hasNext()) {
    const auto filename = it.next();
    if (filename.startsWith(prefix)) {
      const int code_length = filename.size() - prefix_length - suffix_length;
      if (code_length >= 0) {
        const auto code = filename.mid(prefix_length, code_length);
        language_codes.push_back(code);
      }
    }
  }
  return std::vector(language_codes.begin(), language_codes.end());
}

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
  { Qt::KeypadModifier, QT_TRANSLATE_NOOP("Preferences","Keypad") },
};

}  // namespace

namespace omm
{

GeneralPage::MouseModifiersGroup::
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

void GeneralPage::MouseModifiersGroup::apply()
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

GeneralPage::GeneralPage(Preferences& preferences)
  : m_available_languages(languages())
  , m_preferences(preferences)
{
  auto form_layout = std::make_unique<QFormLayout>();
  {
    auto cb_language = std::make_unique<QComboBox>();
    m_cb_language = cb_language.get();
    for (const QString& code : m_available_languages) {
      const auto language = QLocale(code.toUtf8().constData()).language();
      cb_language->addItem(tr(QLocale::languageToString(language).toStdString().c_str()));
    }
    const auto locale = QSettings().value(MainWindow::LOCALE_SETTINGS_KEY).toLocale();
    const auto it = std::find_if(m_available_languages.begin(), m_available_languages.end(),
                                 [&locale](const QString& code)
    {
      return QLocale(code.toUtf8().constData()).language() == locale.language();
    });
    assert(it != m_available_languages.end());
    const int i = std::distance(m_available_languages.begin(), it);
    cb_language->setCurrentIndex(i);
    connect(cb_language.get(), qOverload<int>(&QComboBox::currentIndexChanged), this, [this]()
    {
      const auto msg = tr("Changing language takes effect after restarting the application.");
      QMessageBox::information(this, MainWindow::tr("information"), msg);
    });
    form_layout->addRow(tr("&Language:"), cb_language.release());
  }

  auto grid_layout = std::make_unique<QGridLayout>();
  {
    int row = 0;
    for (auto& [key, mm] : m_preferences.mouse_modifiers()) {
      m_mouse_modifiers.insert({key, MouseModifiersGroup(mm, *grid_layout, row)});
    }
  }

  auto layout = std::make_unique<QVBoxLayout>();
  layout->addLayout(form_layout.release());
  layout->addWidget([]() {
    auto line = std::make_unique<QFrame>();
    line->setFrameShape(QFrame::HLine);
    return line.release();
  }());
  layout->addLayout(grid_layout.release());
  layout->addStretch();

  setLayout(layout.release());

}

GeneralPage::~GeneralPage()
{
}

void GeneralPage::about_to_reject()
{
}

void GeneralPage::about_to_accept()
{
  const QLocale locale(m_available_languages.at(m_cb_language->currentIndex()));
  QSettings().setValue(MainWindow::LOCALE_SETTINGS_KEY, locale);

  for (auto&& [ key, value ] : m_mouse_modifiers) {
    value.apply();
  }
}

}  // namespace omm
