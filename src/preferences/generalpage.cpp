#include "preferences/generalpage.h"

#include <QDirIterator>
#include "logging.h"
#include <QMessageBox>
#include <qsettings.h>
#include "ui_generalpage.h"
#include "mainwindow/mainwindow.h"

namespace
{

std::vector<std::string> languages()
{
  static constexpr auto base_path = ":/qm";
  static constexpr auto prefix = ":/qm/omm_";
  static constexpr auto prefix_length = std::string_view(prefix).length();
  static constexpr auto suffix = ".qm";
  static constexpr auto suffix_length = std::string_view(suffix).length();

  std::list<std::string> language_codes;

  QDirIterator it(base_path);
  while (it.hasNext()) {
    const auto filename = it.next();
    if (filename.startsWith(prefix)) {
      const int code_length = filename.size() - prefix_length - suffix_length;
      if (code_length >= 0) {
        const auto code = filename.mid(prefix_length, code_length);
        language_codes.push_back(code.toStdString());
      }
    }
  }
  return std::vector(language_codes.begin(), language_codes.end());
}

}  // namespace

namespace omm
{

GeneralPage::GeneralPage()
  : m_ui(new Ui::GeneralPage)
  , m_available_languages(languages())
{
  m_ui->setupUi(this);
  for (const std::string& code : m_available_languages) {
    const auto language = QLocale(code.c_str()).language();
    m_ui->cb_language->addItem(tr(QLocale::languageToString(language).toStdString().c_str()));
  }
  const auto locale = QSettings().value(MainWindow::LOCALE_SETTINGS_KEY).toLocale();
  const auto it = std::find_if(m_available_languages.begin(), m_available_languages.end(),
                               [&locale](const std::string& code)
  {

    return QLocale(code.c_str()).language() == locale.language();
  });
  assert(it != m_available_languages.end());
  const int i = std::distance(m_available_languages.begin(), it);
  m_ui->cb_language->setCurrentIndex(i);
  connect(m_ui->cb_language, qOverload<int>(&QComboBox::currentIndexChanged), this, [this]()
  {
    const auto msg = tr("Changing language takes effect after restarting the application.");
    QMessageBox::information(this, MainWindow::tr("information"), msg);
  });
}

GeneralPage::~GeneralPage()
{
}

void GeneralPage::about_to_reject()
{

}

void GeneralPage::about_to_accept()
{
  const QLocale locale(m_available_languages.at(m_ui->cb_language->currentIndex()).c_str());
  QSettings().setValue(MainWindow::LOCALE_SETTINGS_KEY, locale);
}

}  // namespace omm
