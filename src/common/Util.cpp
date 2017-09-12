#include "Util.h"
#include <QStringList>
#include <QDir>
#include <QDirIterator>
#include <QDebug>
#include "hanzi2pinyin.h"
#include <windows.h>
#include <QWidget>
#include <QApplication>

namespace Util
{

	QStringList getFiles(QString path, bool containsSubDir)
	{
		QStringList result;
		QDirIterator curit(path, QStringList(), QDir::Files);
		while (curit.hasNext()) {
			result.push_back(curit.next());
		}

		if (containsSubDir) {
			QDirIterator subit(path, QStringList(), QDir::Files, QDirIterator::Subdirectories);
			while (subit.hasNext()) {
				result.push_back(subit.next());
			}
		}

		return result;
	}

	QString getPinyin(const QString &text)
	{
		std::wstring result = L"";
		std::wstring wstr = text.toStdWString();
		for (auto iter = wstr.begin(); iter != wstr.end(); ++iter) {
			result += retrievePinyin(*iter);
		}
		return QString::fromStdWString(result);
	}

	QPair<QString, QString> getPinyinAndJianpin(const QString &text)
	{
		static const QStringList s_dyz1 = QStringList()
			<< QStringLiteral("�� �� ��")
			<< QStringLiteral("�� ��")
			<< QStringLiteral("Ϻ ��");
		static const QStringList s_dyz2 = QStringList()
			<< "ai qi yi"
			<< "yin yue"
			<< "xia mi";
		Q_ASSERT(s_dyz1.size() == s_dyz2.size());

		QStringList strList = text.split("", QString::SkipEmptyParts);
		QString spaceText = strList.join(" ");
		for (int i = 0; i < s_dyz1.size(); i++) {
			if (spaceText.contains(s_dyz1[i])) {
				spaceText.replace(s_dyz1[i], s_dyz2[i]);
			}
		}

		strList = spaceText.split(" ", QString::SkipEmptyParts);
		QString pinyin, jianpin;
		for (auto iter = strList.begin(); iter != strList.end(); ++iter) {
			QString tmp = getPinyin(*iter);
			if (tmp.size() > 0) {
				pinyin += tmp;
				jianpin += tmp[0];
			}
		}
		return qMakePair(pinyin, jianpin);
	}

	bool shellExecute(const QString &path)
	{
		HINSTANCE hinst = ::ShellExecute(nullptr, L"open", path.toStdWString().c_str(), NULL, NULL, SW_SHOWNORMAL);
		LONG64 result = (LONG64)hinst;
		if (result <= 32) {
			qDebug() << "shellExecute failed, code:" << result;
			return false;
		}
		return true;
	}

	bool locateFile(const QString &dir)
	{
		QString newDir = dir;
		QString cmd = QString("/select, \"%1\"").arg(newDir.replace("/", "\\"));
		qDebug() << cmd;
		HINSTANCE hinst = ::ShellExecute(NULL, L"open", L"explorer.exe", cmd.toStdWString().c_str(), NULL, SW_SHOW);
		LONG64 result = (LONG64)hinst;
		if (result <= 32) {
			qDebug() << "locateFile failed, code:" << result;
			return false;
		}
		return true;
	}

	void setForegroundWindow(QWidget *widget)
	{
		if (widget) {
			HWND hwnd = (HWND)widget->winId();
			::SetForegroundWindow(hwnd);
		}
	}

	void showWndTopMost(QWidget *widget)
	{
		if (widget) {
			HWND hwnd = (HWND)widget->winId();
			RECT rect;
			GetWindowRect(hwnd, &rect);
			SetWindowPos(hwnd, HWND_TOPMOST, rect.left, rect.top, abs(rect.right - rect.left), abs(rect.bottom - rect.top), SWP_SHOWWINDOW);
		}
	}

	void cancelTopMost(QWidget *widget)
	{
		if (widget) {
			HWND hwnd = (HWND)widget->winId();
			RECT rect;
			GetWindowRect(hwnd, &rect);
			SetWindowPos(hwnd, HWND_NOTOPMOST, rect.left, rect.top, abs(rect.right - rect.left), abs(rect.bottom - rect.top), SWP_SHOWWINDOW);
		}
	}

	QPixmap img(const QString &name)
	{
		QString path = ":/images/" + name;
		return QPixmap(path);
	}

	QString getRunDir()
	{
		return QApplication::applicationDirPath();
	}

	QString getConfigDir()
	{
		QDir configDir(getRunDir() + "/config");
		if (!configDir.exists()) {
			configDir.mkdir("config");
		}
		return configDir.absolutePath();
	}

	QString getConfigPath()
	{
		return getConfigDir() + "/base.ini";
	}
}