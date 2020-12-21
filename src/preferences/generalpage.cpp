#include "preferences/generalpage.h"

#include "logging.h"
#include "mainwindow/mainwindow.h"
#include "ui_generalpage.h"
#include <QDirIterator>
#include <QMessageBox>
#include <QSettings>

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
      const int code_length = static_cast<int>(filename.size()) - static_cast<int>(prefix_length)
                              - static_cast<int>(suffix_length);
      if (code_length >= 0) {
        const auto code = filename.mid(prefix_length, code_length);
        language_codes.push_back(code);
      }
    }
  }
  return std::vector(language_codes.begin(), language_codes.end());
}

}  // namespace

namespace omm
{
GeneralPage::GeneralPage(Preferences& preferences)
    : m_available_languages(languages()), m_preferences(preferences), m_ui(new Ui::GeneralPage)
{
  m_ui->setupUi(this);
  for (const QString& code : m_available_languages) {
    const auto language = QLocale(code.toUtf8().constData()).language();
    m_ui->cb_language->addItem(tr(QLocale::languageToString(language).toStdString().c_str()));
  }
  const auto locale = QSettings().value(MainWindow::LOCALE_SETTINGS_KEY).toLocale();
  const auto it
      = std::find_if(m_available_languages.begin(),
                     m_available_languages.end(),
                     [&locale](const QString& code) {
                       return QLocale(code.toUtf8().constData()).language() == locale.language();
                     });

  m_ui->cb_language->setCurrentIndex([it, this]() -> int {
    if (it != m_available_languages.end()) {
      return std::distance(m_available_languages.begin(), it);
    } else {
      return -1;
    }
  }());

  connect(m_ui->cb_language, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]() {
    const auto msg = tr("Changing language takes effect after restarting the application.");
    QMessageBox::information(this, MainWindow::tr("information"), msg);
  });
}

GeneralPage::~GeneralPage() = default;

void GeneralPage::about_to_accept()
{
  if (const int i = m_ui->cb_language->currentIndex(); i >= 0) {
    const QLocale locale(m_available_languages.at(i));
    QSettings().setValue(MainWindow::LOCALE_SETTINGS_KEY, locale);
  }
}

}  // namespace omm
