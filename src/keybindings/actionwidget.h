#include <QWidget>

class QLabel;

namespace omm
{

class PreferencesTreeValueItem;
class ActionWidget : public QWidget
{
public:
  ActionWidget(QWidget* parent, const PreferencesTreeValueItem& key_binding);
  void set_highlighted(bool h);
  void showEvent(QShowEvent *event) override;

private:
  QLabel* m_name_label;
  QLabel* m_shortcut_label;

};

}  // namespace omm