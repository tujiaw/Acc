#pragma once

#include <QFrame>
#include "SystemTray.h"

class QxtGlobalShortcut;
class LnkListView;
class QLineEdit;
class MainWidget : public QFrame
{
	Q_OBJECT

public:
	MainWidget(QWidget *parent);
	~MainWidget();

protected:
	bool eventFilter(QObject *, QEvent *);

private slots:
	void slotReload();
	void slotTrayActivated(QSystemTrayIcon::ActivationReason reason);
	void slotMainShortcut();
	void slotMainShortcutChanged(const QString &textKey);
	void slotSearchTimer();

private:
	SystemTray *tray_;
	QxtGlobalShortcut *mainShortcut_;
	QLineEdit *m_lineEdit;
	LnkListView *m_lnkListView;
	QTimer *m_searchTimer;
};
