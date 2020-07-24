#pragma once

#include <QObject>
#include <string>
#include <vector>
#include "Constants.h"

class QWidget;
namespace Util {
    void getFiles(const std::string &folder, std::vector<std::string> &outFiles);
    void getFiles(const std::wstring &folder, std::vector<std::wstring> &outFiles);
	bool shellExecute(const QString &path);
    bool shellExecute(const QString &path, const QString &operation);
	bool locateFile(const QString &dir);
	void setForegroundWindow(QWidget *widget);
	void showWndTopMost(QWidget *widget);
	void cancelTopMost(QWidget *widget);
	QPixmap img(const QString &name);
	QString getRunDir();
    QString getWritebaleDir();
    QFileInfoList getRecentFileList();
	QString getConfigDir();
	QString getConfigPath();
    QString getIndexDir(const QString &name = "");
    QString getLogsDir();
    bool removeDir(const QString &dir, bool containSubDir = true);
    bool clearDir(const QString &dir, bool containSubDir = true);
    QString getImagesDir();
	QString getSystemDir(int csidl);
    QStringList getAllLnkDir();
	QStringList getAllLnk();
	QVariantMap json2map(const QByteArray &val);
	QString map2json(const QVariantMap &val);
	QVariantList json2list(const QByteArray &val);
	QString list2json(const QVariantList &val);
    uint toKey(const QString& str);
    std::string gbk2utf8(const std::string &gbkStr);
    std::string utf82gbk(const std::string &utf8Str);
    void setWallpaper(const QString &imagePath);
    void setWallpaperBMP(const QString &imagePath);
    QString md5(const QString &str);
    QString getTimeInterval(quint64 msInterval);
    bool ImproveProcPriv();
	QString getSystemInfo();
	QString getLocalHost();
    void convert(const std::vector<std::string> &in, QStringList &out);
    void convert(const std::vector<std::wstring> &in, QStringList &out);
}
