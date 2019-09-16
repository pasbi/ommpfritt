#pragma once

#include <QWidget>
#include <set>

namespace omm
{

class AbstractPropertyOwner;
class Animator;
class AnimationButton : public QWidget
{
  Q_OBJECT
public:
  AnimationButton(Animator& animator, const std::set<AbstractPropertyOwner*>& owners,
                  const std::string& property_key, QWidget* parent = nullptr);

  bool has_key() const;
  bool has_track() const;

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
  const std::set<AbstractPropertyOwner*> m_owners;
  const std::string m_property_key;
};

}  // namespace omm
