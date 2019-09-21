#pragma once

#include <QWidget>
#include <set>

namespace omm
{

class Property;
class Animator;
class AnimationButton : public QWidget
{
  Q_OBJECT
public:
  AnimationButton(Animator& animator, const std::set<Property*>& properties,
                  QWidget* parent = nullptr);

  bool has_key() const;
  bool has_track() const;
  bool value_is_inconsistent() const;

public Q_SLOTS:
  void set_key();
  void remove_key();
  void remove_track();

Q_SIGNALS:
  void clicked();

protected:
  void resizeEvent(QResizeEvent *event) override;
  void paintEvent(QPaintEvent *event) override;
  void contextMenuEvent(QContextMenuEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;

private:
  Animator& m_animator;
  const std::set<Property*> m_properties;
};

}  // namespace omm
