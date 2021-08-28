#pragma once

#include <QTranslator>

namespace omm
{

class Translator
{
public:
  explicit Translator(const QLocale& locale);

private:
  QTranslator m_omm_translator;
  QTranslator m_qtbase_translator;
};

}  // namespace omm
