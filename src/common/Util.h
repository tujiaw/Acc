#pragma once

#include <QObject>
#include <string>
#include "Constants.h"

class QWidget;
namespace Util {
	QStringList getFiles(QString path, bool containsSubDir = true);
	bool shellExecute(const QString &path);
    bool shellExecute(const QString &path, const QString &operation);
	bool locateFile(const QString &dir);
	void setForegroundWindow(QWidget *widget);
	void showWndTopMost(QWidget *widget);
	void cancelTopMost(QWidget *widget);
	QPixmap img(const QString &name);
	QString getRunDir();
	QString getConfigDir();
	QString getConfigPath();
    QString getImagesDir();
	QString getSystemDir(int csidl);
	QStringList getAllLnk();
	QVariantMap json2map(const QByteArray &val);
	QString map2json(const QVariantMap &val);
	QVariantList json2list(const QByteArray &val);
	QString list2json(const QVariantList &val);
    uint toKey(const QString& str);
    std::string gbk2utf8(const std::string &gbkStr);
    std::string utf82gbk(const std::string &utf8Str);
    void setWallpaper(const QString &imagePath);
}
