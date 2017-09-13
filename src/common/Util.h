#pragma once

#include <QObject>
#include "Constants.h"

class QWidget;
namespace Util {
	QStringList getFiles(QString path, bool containsSubDir = true);
	QString getPinyin(const QString &text);
	QPair<QString, QString> getPinyinAndJianpin(const QString &text);
	bool shellExecute(const QString &path);
	bool locateFile(const QString &dir);
	void setForegroundWindow(QWidget *widget);
	void showWndTopMost(QWidget *widget);
	void cancelTopMost(QWidget *widget);
	QPixmap img(const QString &name);
	QString getRunDir();
	QString getConfigDir();
	QString getConfigPath();
	QString getSystemDir(int csidl);
	QVariantMap json2map(const QByteArray &val);
	QString map2json(const QVariantMap &val);
	QVariantList json2list(const QByteArray &val);
	QString list2json(const QVariantList &val);
}
