#pragma once

#include <map>
#include <QString>
#include <QObject>
#include <Qt>
#include <QColor>

namespace omm
{

class Preferences : public QObject
{
  Q_OBJECT
public:
  Preferences();
  ~Preferences();

  struct MouseModifier
  {
    MouseModifier(const QString& label,
                  Qt::MouseButton default_button, Qt::KeyboardModifiers default_modifiers);
    const QString label;
    Qt::MouseButton button;
    Qt::KeyboardModifiers modifiers;
  };

  struct GridOption
  {
    GridOption(const QString& label, const QColor& color, double pen_width);
    const QString label;
    QColor color;
    double pen_width;
  };

  std::map<QString, MouseModifier> mouse_modifiers;
  std::map<double, GridOption> grid_options;

};

}  // namespace omm
