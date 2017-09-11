#pragma once

#include <QSystemTrayIcon>

class QAction;
class SystemTray : public QSystemTrayIcon
{
	Q_OBJECT

public:
	SystemTray(QWidget *parent);
	~SystemTray();

signals:
	void sigReload();

private slots:
	void slotTriggered(QAction *action);

private:
	QWidget *parent_;
	QMenu *menu_;
};
