#pragma once

#include <QWidget>

namespace omm
{

class AnimationButton : public QWidget
{
  Q_OBJECT
public:
  enum class State { NotAnimated, Animated, KeyValue };
  explicit AnimationButton(QWidget* parent = nullptr);

  void set_state(State state);

Q_SIGNALS:
  void clicked();

protected:
  void resizeEvent(QResizeEvent *event) override;
  void paintEvent(QPaintEvent *event) override;
  void contextMenuEvent(QContextMenuEvent *event) override;

private:
  State m_state = State::NotAnimated;

};

}  // namespace omm
