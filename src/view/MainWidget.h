#pragma once

#include <QFrame>
#include "SystemTray.h"

class QxtGlobalShortcut;
class LnkListView;
class QLineEdit;
class HttpRequest;
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
	void slotClearResult();
    void slotHttpResponse(int err, const QByteArray &data);
	void slotSearchTimer();
    void slotWallpaper();
	void slotTextChanged(const QString &text);
	void slotReturnPressed();

private:
	QString getUrl() const;
	QPair<QString, QString> getPrefixAndText() const;

private:
	SystemTray *tray_;
	QxtGlobalShortcut *mainShortcut_;
	QLineEdit *m_lineEdit;
	LnkListView *m_lnkListView;
	QTimer *m_searchTimer;
    QTimer *m_wallpaperTimer;
    HttpRequest *m_http;
};
