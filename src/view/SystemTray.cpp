#include "SystemTray.h"
#include <QtWidgets>
#include "controller/Acc.h"

SystemTray::SystemTray(QWidget *parent)
	: QSystemTrayIcon(parent)
{
	this->setIcon(QIcon(":/images/Acc.ico"));
	menu_ = new QMenu(parent);
	connect(menu_, &QMenu::triggered, this, &SystemTray::slotTriggered);
	menu_->addAction(tr("Open"));
	menu_->addAction(tr("Setting"));
    menu_->addAction(tr("Clipboard"));
    menu_->addAction(tr("Tools"));
	menu_->addAction(tr("Reload"));
	menu_->addAction(tr("Exit"));
	this->setContextMenu(menu_);
}

SystemTray::~SystemTray()
{
}

void SystemTray::slotTriggered(QAction *action)
{
	QString text = action->text();
	if (text == tr("Open")) {
		Acc::instance()->openWidget(WidgetID::MAIN);
	} else if (text == tr("Setting")) {
		Acc::instance()->openWidget(WidgetID::SETTING);
    } else if (text == tr("Clipboard")) {
        Acc::instance()->openWidget(WidgetID::CLIPBOARD);
    } else if (text == tr("Tools")) {
        Acc::instance()->openWidget(WidgetID::TOOLS);
    } else if (text == tr("Reload")) {
		emit sigReload();
	} else if (text == tr("Exit")) {
		qApp->exit();
	}
}
