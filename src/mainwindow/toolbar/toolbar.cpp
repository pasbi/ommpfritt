#include "mainwindow/toolbar/toolbar.h"
#include "mainwindow/toolbar/toolbardialog.h"
#include <QContextMenuEvent>
#include <QToolButton>
#include "tools/toolbox.h"
#include "mainwindow/application.h"
#include "mainwindow/iconprovider.h"
#include <QApplication>

namespace omm
{

ToolBar::ToolBar(const QString& tools)
{
  setAttribute(Qt::WA_DeleteOnClose);
  configure(tools);
}

void ToolBar::configure(const QString& tools)
{
  m_tools = tools;
  clear();
  if (!tools.isEmpty()) {
    auto& app = Application::instance();
    for (QString token : split(tools)) {
      if (token.startsWith(ToolBar::group_identifiers.first)) {
        assert(token.endsWith(ToolBar::group_identifiers.second));
        token = token.mid(1, token.size() - 2);
        auto button = std::make_unique<QToolButton>();
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        connect(button.get(), SIGNAL(triggered(QAction*)),
                button.get(), SLOT(setDefaultAction(QAction*)));
        for (const QString& action : split(token)) {
          button->addAction(app.key_bindings.make_menu_action(app, action).release());
        }
        button->setDefaultAction(button->actions().first());
        addWidget(button.release());
      } else if (token == ToolBar::separator_identifier) {
        addSeparator();
      } else {
        addAction(app.key_bindings.make_toolbar_action(app, token).release());
      }
    }
  }
  auto spacer = std::make_unique<QWidget>();
  spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  addWidget(spacer.release());
  connect(addAction(tr("E")), &QAction::triggered, [this]() {
    ToolBarDialog dialog(this->tools());
    if (dialog.exec() == QDialog::Accepted) {
      configure(dialog.tools());
    }
  });
}

QStringList ToolBar::split(const QString& string)
{
  if (string.isEmpty()) {
    return {};
  } else {
    QStringList list;
    int level = 0;
    int last = 0;
    int i = 0;
    for (i = 0; i < string.size(); ++i) {
      if (string.at(i) == group_identifiers.first) {
        level += 1;
      } else if (string.at(i) == group_identifiers.second) {
        level -= 1;
        if (level < 0) {
          LWARNING << "no matching open group identifier.";
          level = 0;
        }
      } else if (level == 0 && string[i] == separator) {
        list.push_back(string.mid(last, i - last));
        last = i + 1;
      }
    }
    if (level != 0) {
      LWARNING << "no matching closing group identifier.";
    }
    list.push_back(string.mid(last, i - last));
    return list;
  }
}

}  // namespace omm
