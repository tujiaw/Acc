#pragma once

#include <QObject>

class QWidget;
namespace Util {

	QStringList getFiles(QString path, bool containsSubDir = true);
	QString getPinyin(const QString &text);
	QPair<QString, QString> getPinyinAndJianpin(const QString &text);
	bool shellExecute(const QString &path);
	void setForegroundWindow(QWidget *widget);
	void showWndTopMost(QWidget *widget);
	void cancelTopMost(QWidget *widget);
}
