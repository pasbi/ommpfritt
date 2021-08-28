#include "translator.h"

#include <QCoreApplication>
#include <QDebug>
#include <QLocale>

namespace
{

void load_translator(const QLocale& locale, const QString& name, QTranslator& translator)
{
  if (translator.load(locale, name, "_", ":/qm")) {
    QCoreApplication::installTranslator(&translator);
  } else {
    qCritical() << "Failed to load " << name << " translator for " << locale;
  }
}

}  //  namespace

namespace omm
{

Translator::Translator(const QLocale& locale)
{
  load_translator(locale, "omm", m_omm_translator);
  load_translator(locale, "qtbase", m_qtbase_translator);
}

}  // namespace omm
