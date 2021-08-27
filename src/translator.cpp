#include "translator.h"

#include <QCoreApplication>
#include <QDebug>
#include "config.h"

namespace
{

bool load_translator(const QLocale& locale, const QString& name, const QString& path, QTranslator& translator)
{
  if (translator.load(locale, name, "_", path)) {
    QCoreApplication::installTranslator(&translator);
    return true;
  } else {
    qInfo() << "Failed to load " << name << " translator for " << locale << " in " << path;
    return false;
  }
}

bool load_local_translator(const QLocale& locale, const QString& name, QTranslator& translator)
{
  static const auto local_qm_path = QCoreApplication::applicationDirPath() + "/../qm";
  return load_translator(locale, name, local_qm_path, translator);
}

bool load_global_translator(const QLocale& locale, const QString& name, QTranslator& translator)
{
  return load_translator(locale, name, qt_qm_path, translator);
}

}  //  namespace

namespace omm
{

Translator::Translator(const QLocale& locale)
{
  if (!load_local_translator(locale, "omm", m_omm_translator)) {
    qCritical() << "No translator for omm available for " << locale;
  }

  if (!load_local_translator(locale, "qtbase", m_qtbase_translator)
      && !load_global_translator(locale, "qtbase", m_qtbase_translator))
  {
    qCritical() << "No translator for qtbase available for " << locale;
  }
}

}  // namespace omm
